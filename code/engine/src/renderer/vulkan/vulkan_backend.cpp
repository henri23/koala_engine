#include "renderer/renderer_types.inl"
#include "renderer/vulkan/vulkan_device.hpp"
#include "vulkan_types.inl"

#include "core/asserts.hpp"
#include "core/logger.hpp"
#include "core/string.hpp"

#include "containers/auto_array.hpp"

#include <vulkan/vulkan_core.h>

internal Vulkan_Context context;

// Forward declare messenger callback
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

b8 vulkan_create_debug_logger(
    VkInstance* instance);

b8 vulkan_enable_validation_layers(
    Auto_Array<const char*>* required_layers_array);

b8 vulkan_initialize(
    Renderer_Backend* backend,
    const char* app_name,
    struct Platform_State* plat_state) {

    context.allocator = nullptr;

    VkApplicationInfo app_info =
        {VK_STRUCTURE_TYPE_APPLICATION_INFO};

    app_info.pNext = nullptr;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Koala engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    // CreateInfo struct tells the Vulkan driver which global extensions and validation
    // layers to use.
    VkInstanceCreateInfo createInfo =
        {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

    createInfo.pApplicationInfo = &app_info;

    Auto_Array<const char*> required_extensions_array;

    // Get list of required extensions
    required_extensions_array.add(VK_KHR_SURFACE_EXTENSION_NAME);

    // Get platform specific extensions
#if ENGINE_PLATFORM_LINUX
    ENGINE_INFO("Attaching XCB surface for LINUX platform");
    required_extensions_array.add("VK_KHR_xcb_surface");
#elif ENGINE_PLATFORM_WINDOWS
    ENGINE_INFO("Attaching WIN32 surface for Windows platform");
    required_extensions_array.add("VK_KHR_win32_surface");
#endif

    Auto_Array<const char*> required_layers_array;

// Only enable validation layer in debug builds
#ifdef DEBUG_BUILD 
    // Add debug extensions
    required_extensions_array.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    ENGINE_DEBUG("Required VULKAN extensions:");
    for (u32 i = 0; i < required_extensions_array.length; ++i) {
        ENGINE_DEBUG(required_extensions_array[i]);
    }

    // Add validation layers
    vulkan_enable_validation_layers(&required_layers_array);
#endif

    createInfo.enabledExtensionCount = required_extensions_array.length;
    createInfo.ppEnabledExtensionNames = required_extensions_array.data;
    createInfo.enabledLayerCount = required_layers_array.length;
    createInfo.ppEnabledLayerNames = required_layers_array.data;

    VK_ENSURE_SUCCESS(
        vkCreateInstance(&createInfo,
                         context.allocator,
                         &context.instance));

    required_extensions_array.free();
    required_layers_array.free();

#ifdef DEBUG_BUILD
    // Depends on the instance
    vulkan_create_debug_logger(&context.instance);
#endif

    Auto_Array<const char*> extension_requirements;

    extension_requirements.add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Setup Vulkan device
    Vulkan_Physical_Device_Requirements device_requirements;
    device_requirements.compute = TRUE;
    device_requirements.sampler_anisotropy = TRUE;
    device_requirements.graphics = TRUE;
    device_requirements.transfer = TRUE;
    device_requirements.present = FALSE; // TODO: Setup present queue lookup
    device_requirements.discrete_gpu = TRUE;
    device_requirements.device_extension_names = &extension_requirements;

    if (!vulkan_device_initialize(&context, &device_requirements)) {
        ENGINE_FATAL("No device that fulfills all the requirements was found in the machine");
        return FALSE;
    }

    ENGINE_INFO("Vulkan backend initialized");

    extension_requirements.free();

    return TRUE;
}

void vulkan_shutdown(Renderer_Backend* backend) {

#ifdef DEBUG_BUILD
    ENGINE_DEBUG("Destroying Vulkan debugger...");
    if (context.debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(context.instance,
                                      "vkDestroyDebugUtilsMessengerEXT");

        func(context.instance,
             context.debug_messenger,
             context.allocator);
    }
#endif

    vkDestroyInstance(
        context.instance,
        context.allocator);

    ENGINE_DEBUG("Vulkan renderer shut down");
}

void vulkan_on_resized(Renderer_Backend* backend,
                       u16 width,
                       u16 height) {
}

b8 vulkan_begin_frame(Renderer_Backend* backend,
                      f32 delta_t) {
    return TRUE;
}

b8 vulkan_end_frame(Renderer_Backend* backend,
                    f32 delta_t) {
    return TRUE;
}

b8 vulkan_enable_validation_layers(
    Auto_Array<const char*>* required_layers_array) {

    ENGINE_INFO("Vulkan validation layers enabled. Enumerating...");

    // Declare the list of layers that we require
    required_layers_array->add("VK_LAYER_KHRONOS_validation");

    // Need to check whether the validation layer requuested is supported
    u32 available_layer_count;

    // To allocate the array needed for storing the available layers,
    // first I need to know how many layers there are
    VK_ENSURE_SUCCESS(
        vkEnumerateInstanceLayerProperties(&available_layer_count,
                                           nullptr));

    VkLayerProperties available_layers_array[available_layer_count];

    VK_ENSURE_SUCCESS(
        vkEnumerateInstanceLayerProperties(&available_layer_count,
                                           available_layers_array));

    // ENGINE_DEBUG("Available Vulkan layers:");
    // for (u32 i = 0; i < available_layer_count; ++i) {
    //     ENGINE_DEBUG(available_layers_array[i].layerName);
    // }

    for (u32 i = 0; i < required_layers_array->length; ++i) {
        ENGINE_INFO("Searching for layer: %s ...",
                    required_layers_array->data[i]);

        b8 found = FALSE;
        for (u32 j = 0; j < available_layer_count; ++j) {
            if (string_check_equal(
                    required_layers_array->data[i],
                    available_layers_array[j].layerName)) {
                found = TRUE;
                ENGINE_INFO("Found.");
                break;
            }
        }

        if (!found) {
            ENGINE_FATAL("Required validation layer is missing: %s",
                         required_layers_array->data[i]);
            return FALSE;
        }
    }

    ENGINE_INFO("All required validaton layers are valid");
    return TRUE;
}

b8 vulkan_create_debug_logger(VkInstance* instance) {

    ENGINE_DEBUG("Creating Vulkan debug logger");

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info =
        {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

    debug_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    debug_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    debug_create_info.pfnUserCallback = vk_debug_callback;
    debug_create_info.pUserData = nullptr; // Optional pointer that can be passed to the logger

    // The vkCreateDebugUtilsMessengerEXT is an extension function so it is not
    // loaded automatically. Its address must be looked up manually
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(*instance,
                                  "vkCreateDebugUtilsMessengerEXT");

    RUNTIME_ASSERT_MSG(func, "Failed to create debug messenger");

    VK_ENSURE_SUCCESS(
        func(*instance, // Pass instance because debugger is specific for inst.
             &debug_create_info,
             context.allocator,
             &context.debug_messenger));

    ENGINE_DEBUG("Vulkan debugger created");
    return TRUE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {

    switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ENGINE_ERROR(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        ENGINE_WARN(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        ENGINE_INFO(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        ENGINE_TRACE(callback_data->pMessage);
        break;
    default:
    }
    return VK_FALSE;
}
