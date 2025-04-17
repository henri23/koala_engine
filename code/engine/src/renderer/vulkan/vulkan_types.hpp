#pragma once

#include "core/asserts.hpp"
#include "defines.hpp"

#include "containers/auto_array.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define VK_ENSURE_SUCCESS(expr) RUNTIME_ASSERT(expr == VK_SUCCESS);

struct Vulkan_Swapchain_Support_Info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formats_count;
    VkSurfaceFormatKHR* formats;
    u32 present_modes_count;
    VkPresentModeKHR* present_modes;
};

struct Vulkan_Device {
    VkPhysicalDevice physical_device; // Handle ptr to physical device
    VkDevice logical_device;          // Handle to be destroyed

    // Family queue indices
    u32 graphics_queue_index;
    u32 transfer_queue_index;
    u32 compute_queue_index;
    u32 present_queue_index;

    // Physical device informations
    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceFeatures physical_device_features;
    VkPhysicalDeviceMemoryProperties physical_device_memory;

    Vulkan_Swapchain_Support_Info swapchain_info;
    // Queue handles
    VkQueue presentation_queue;
    VkQueue graphics_queue;
    VkQueue transfer_queue;
};

struct Vulkan_Context {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkAllocationCallbacks* allocator;
    VkPhysicalDevice physical_device; // Implicitly destroyed destroying VkInstance

#ifdef DEBUG_BUILD
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    Vulkan_Device device;
};

struct Vulkan_Physical_Device_Requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    b8 discrete_gpu;
    b8 sampler_anisotropy;
    Auto_Array<const char*>* device_extension_names;
};

#define VK_DEVICE_LEVEL_FUNCTION(device, name)             \
    PFN_##name name = (PFN_##name)vkGetDeviceProcAddr(device, #name); \
    RUNTIME_ASSERT_MSG(name, "Could not load device-level Vulkan function");

#define VK_INSTANCE_LEVEL_FUNCTION(instance, name)             \
    PFN_##name name = (PFN_##name)vkGetInstanceProcAddr(instance, #name); \
    RUNTIME_ASSERT_MSG(name, "Could not load instance-level Vulkan function");\
