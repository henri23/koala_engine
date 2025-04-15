#include "vulkan_device.hpp"

#include <vulkan/vulkan_core.h>

#include "core/logger.hpp"
#include "core/memory.hpp"
#include "core/string.hpp"
#include "renderer/vulkan/vulkan_types.hpp"

struct Device_Queue_Indices {
    u32 graphics_family_index;
    u32 transfer_family_index;
    u32 present_family_index;
    u32 compute_family_index;
};

b8 is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface,
                      const VkPhysicalDeviceProperties* properties,
                      const VkPhysicalDeviceFeatures* features,
                      const Vulkan_Physical_Device_Requirements* requirements,
                      Vulkan_Swapchain_Support_Info* out_swapchain_info,
                      Device_Queue_Indices* out_indices);

b8 select_physical_device(Vulkan_Context* context,
                          Vulkan_Physical_Device_Requirements* requirements);

b8 create_logical_device(Vulkan_Context* context);

void vulkan_device_query_swapchain_capabilities(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    Vulkan_Swapchain_Support_Info* out_swapchain_info);

b8 vulkan_device_initialize(Vulkan_Context* context,
                            Vulkan_Physical_Device_Requirements* requirements) {
    // Select physical device in the machine
    if (!select_physical_device(context, requirements)) {
        ENGINE_FATAL("Failed to select physical device. Aborting...");
        return FALSE;
    }

    if (!create_logical_device(context)) {
        ENGINE_FATAL("Failed to create logical device. Aborting...");
        return FALSE;
    }

    // Create logical device

    return TRUE;
}

b8 select_physical_device(Vulkan_Context* context,
                          Vulkan_Physical_Device_Requirements* requirements) {
    u32 physical_device_count = 0;

    // Retrieve the list of available GPUs with vulkan support
    vkEnumeratePhysicalDevices(context->instance, &physical_device_count,
                               nullptr);

    // Check if there's at least one GPU that supports Vulkan
    if (physical_device_count == 0)
        return FALSE;

    VkPhysicalDevice physical_devices_array[physical_device_count];

    vkEnumeratePhysicalDevices(context->instance, &physical_device_count,
                               physical_devices_array);

    // Evaluate GPUs -> If mutliple GPUs are present in the machine,
    // we need to pick the most "qualified" one
    for (u32 i = 0; i < physical_device_count; ++i) {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        VkPhysicalDeviceMemoryProperties device_memory_properties;

        vkGetPhysicalDeviceProperties(physical_devices_array[i],
                                      &device_properties);
        vkGetPhysicalDeviceFeatures(physical_devices_array[i],
                                    &device_features);
        vkGetPhysicalDeviceMemoryProperties(physical_devices_array[i],
                                            &device_memory_properties);

        Device_Queue_Indices queue_indices;

        // Score the GPUs based on the properties they provide
        b8 result = is_device_suitable(
            physical_devices_array[i], context->surface, &device_properties,
            &device_features, requirements, &context->device.swapchain_info,
            &queue_indices);

        if (result) {
            ENGINE_INFO("Selected device: '%s'", device_properties.deviceName);
            switch (device_properties.deviceType) {
            default:
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                ENGINE_INFO("GPU type is unknown.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                ENGINE_INFO("GPU type is discrete.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                ENGINE_INFO("GPU type is integrated.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                ENGINE_INFO("GPU type is CPU.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                ENGINE_INFO("GPU type is virtual.");
                break;
            }

            ENGINE_DEBUG("GPU Driver Version: %d.%d.%d",
                         VK_VERSION_MAJOR(device_properties.driverVersion),
                         VK_VERSION_MINOR(device_properties.driverVersion),
                         VK_VERSION_PATCH(device_properties.driverVersion));

            ENGINE_DEBUG("Vulkan API Version: %d.%d.%d",
                         VK_VERSION_MAJOR(device_properties.apiVersion),
                         VK_VERSION_MINOR(device_properties.apiVersion),
                         VK_VERSION_PATCH(device_properties.apiVersion));

            for (u32 j = 0; j < device_memory_properties.memoryHeapCount; ++j) {
                f32 memory_size =
                    device_memory_properties.memoryHeaps[j].size / (float)GIB;
                if (device_memory_properties.memoryHeaps[j].flags &
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                    ENGINE_DEBUG("Local GPU memory: %.2f GiB", memory_size);
                } else {
                    ENGINE_DEBUG("Shared GPU memory: %.2f GiB", memory_size);
                }
            }

            // Store device handle in the vulkan context
            context->device.physical_device = physical_devices_array[i];
            context->device.physical_device_properties = device_properties;
            context->device.physical_device_features = device_features;
            context->device.physical_device_memory = device_memory_properties;

            // Store indices for queue instantiation later
            context->device.graphics_queue_index =
                queue_indices.graphics_family_index;
            context->device.transfer_queue_index =
                queue_indices.transfer_family_index;
            context->device.compute_queue_index =
                queue_indices.compute_family_index;
            context->device.present_queue_index =
                queue_indices.present_family_index;

            break;
        }
    }

    if (context->device.physical_device) {
        return TRUE;
    }

    return FALSE;
}

b8 create_logical_device(Vulkan_Context* context) {
    ENGINE_INFO("Creating logical device...");

    u32 distinct_queue_family_indices_count =
        1; // At least one for the graphics queue

    b8 does_transfer_share_queue = context->device.transfer_queue_index ==
                                   context->device.graphics_queue_index;
    b8 does_present_share_queue = context->device.present_queue_index ==
                                  context->device.graphics_queue_index;

    if (!does_transfer_share_queue)
        distinct_queue_family_indices_count++;

    if (!does_present_share_queue)
        distinct_queue_family_indices_count++;

    u32 queue_family_indeces[distinct_queue_family_indices_count];

    queue_family_indeces[0] = context->device.graphics_queue_index;

    if (!does_transfer_share_queue)
        queue_family_indeces[1] = context->device.transfer_queue_index;

    if (!does_present_share_queue)
        queue_family_indeces[2] = context->device.present_queue_index;

    VkDeviceQueueCreateInfo
        queue_create_infos[distinct_queue_family_indices_count];
    u32 max_queue_count = 2;

    f32 queue_priorities[2] = {1.0f, 1.0f};
    for (u32 i = 0; i < distinct_queue_family_indices_count; ++i) {
        queue_create_infos[i].sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = queue_family_indeces[i];

        // If we are on the graphics queue, instantiate 2 queues
        queue_create_infos[i].queueCount = (i == 0) ? 2 : 1;

        queue_create_infos[i].pQueuePriorities = queue_priorities;
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = nullptr;
    }

    VkPhysicalDeviceFeatures device_features_to_request = {};
    device_features_to_request.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo logical_device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    logical_device_create_info.pQueueCreateInfos = queue_create_infos;
    logical_device_create_info.queueCreateInfoCount =
        distinct_queue_family_indices_count;
    logical_device_create_info.pEnabledFeatures = &device_features_to_request;

    // Request swapchain extension for physical device
    const char* required_extensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    logical_device_create_info.ppEnabledExtensionNames = &required_extensions;
    logical_device_create_info.enabledExtensionCount = 1;

    // Depracated, for calirity explicitly set them no uninitialized
    logical_device_create_info.enabledLayerCount = 0;
    logical_device_create_info.ppEnabledLayerNames = nullptr;

    VK_ENSURE_SUCCESS(
        vkCreateDevice(
            context->device.physical_device,
            &logical_device_create_info,
            context->allocator, 
			&context->device.logical_device));

    ENGINE_INFO("Logical device created.");

    vkGetDeviceQueue(context->device.logical_device,
                     context->device.graphics_queue_index, 0,
                     &context->device.graphics_queue);

    vkGetDeviceQueue(context->device.logical_device,
                     context->device.transfer_queue_index, 0,
                     &context->device.transfer_queue);

    vkGetDeviceQueue(context->device.logical_device,
                     context->device.present_queue_index, 0,
                     &context->device.presentation_queue);

    ENGINE_INFO("Queues obtained");

    return TRUE;
}

// TODO: 	For now the algorithm just checks if the current GPU fullfills
// 			the requirements, and if so it breaks, so if there are
// mulitple 			GPUs that can fulfill those requirements, the
// first one gets 			selected, not necessarily the best
b8 is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface,
                      const VkPhysicalDeviceProperties* properties,
                      const VkPhysicalDeviceFeatures* features,
                      const Vulkan_Physical_Device_Requirements* requirements,
                      Vulkan_Swapchain_Support_Info* out_swapchain_info,
                      Device_Queue_Indices* out_indices) {
    // Initialize the family index to a unreasonable value so that it is
    // evident whether or not a queue family that supports given commands
    // is found
    out_indices->graphics_family_index = -1;
    out_indices->compute_family_index = -1;
    out_indices->present_family_index = -1;
    out_indices->transfer_family_index = -1;

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             nullptr);

    VkQueueFamilyProperties queue_family_array[queue_family_count];

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             queue_family_array);

    if (requirements->discrete_gpu &&
        properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        ENGINE_DEBUG("Device is not a discrete GPU. Skipping.");
        return FALSE;
    }

    ENGINE_INFO("Graphics | Present | Compute | Transfer | Name");

    // If a queue family offers Transfer commands capability on top of other
    // types of commands, maybe it is not the best possible options, so the
    // target family queue would be a queue "dedicated" to transfer commands.
    // This means that the more additional commands to the transfer ones a
    // queue will have, the less optimal it is to be chosen as for transfer.
    // Obviously if it is the only family queue that provides transfer we will
    // still pick it
    u8 min_transfer_score = 255;

    for (u32 i = 0; i < queue_family_count; ++i) {
        u8 current_transfer_score = 0;

        if (queue_family_array[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_indices->graphics_family_index = i;
            ++current_transfer_score;
        }

        if (queue_family_array[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_indices->compute_family_index = i;
            ++current_transfer_score;
        }

        if (queue_family_array[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
            ++current_transfer_score;

        if (queue_family_array[i].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
            ++current_transfer_score;

        if (queue_family_array[i].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
            ++current_transfer_score;

        // Mark this family as the go to transfer queue family only if it is
        // lower than the current minimum
        if (queue_family_array[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
            current_transfer_score <= min_transfer_score) {
            out_indices->transfer_family_index = i;
            min_transfer_score = current_transfer_score;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &present_support);

        if (present_support) {
            out_indices->present_family_index = i;
        }
    }

    ENGINE_INFO("       %d |       %d |       %d |        %d | %s",
                out_indices->graphics_family_index,
                out_indices->present_family_index,
                out_indices->compute_family_index,
                out_indices->transfer_family_index, properties->deviceName);

    if ((!requirements->graphics ||
         (requirements->graphics &&
          out_indices->graphics_family_index != -1)) &&
        (!requirements->compute ||
         (requirements->compute && out_indices->compute_family_index != -1)) &&
        (!requirements->transfer ||
         (requirements->transfer &&
          out_indices->transfer_family_index != -1)) &&
        (!requirements->present ||
         (requirements->present && out_indices->present_family_index != -1))) {

        vulkan_device_query_swapchain_capabilities(device, surface,
                                                   out_swapchain_info);

        if (out_swapchain_info->formats_count == -1 ||
            out_swapchain_info->present_modes_count == -1) {
            if (out_swapchain_info->formats) {
                memory_deallocate(out_swapchain_info->formats,
                                  sizeof(VkSurfaceFormatKHR) *
                                      out_swapchain_info->formats_count,
                                  Memory_Tag::RENDERER);
            }

            if (out_swapchain_info->present_modes) {
                memory_deallocate(out_swapchain_info->present_modes,
                                  sizeof(VkPresentModeKHR) *
                                      out_swapchain_info->present_modes_count,
                                  Memory_Tag::RENDERER);
            }

            ENGINE_DEBUG("Swapchain is not fully supported. Skipping device.");
            return FALSE;
        }

        ENGINE_INFO("Device '%s' has swapchain support",
                    properties->deviceName);

        ENGINE_INFO("Device meets all the requirements.");

        ENGINE_TRACE("Graphics queue family index: %d",
                     out_indices->graphics_family_index);
        ENGINE_TRACE("Compute queue family index: %d",
                     out_indices->compute_family_index);
        ENGINE_TRACE("Transfer queue family index: %d",
                     out_indices->transfer_family_index);
        ENGINE_TRACE("Present queue family index: %d",
                     out_indices->present_family_index);

        // Check for last whether the device supports all the required ext.
        if (requirements->device_extension_names->length > 0) {
            u32 available_extensions_count = 0;

            VK_ENSURE_SUCCESS(vkEnumerateDeviceExtensionProperties(
                device, nullptr, &available_extensions_count, nullptr));

            // TODO : allocate in HEAP
            VkExtensionProperties
                extension_properties[available_extensions_count];

            VK_ENSURE_SUCCESS(vkEnumerateDeviceExtensionProperties(
                device, nullptr, &available_extensions_count,
                extension_properties));

            for (u32 i = 0; i < requirements->device_extension_names->length;
                 ++i) {
                b8 found = FALSE;

                for (u32 j = 0; j < available_extensions_count; ++j) {
                    if (string_check_equal(
                            extension_properties[j].extensionName,
                            requirements->device_extension_names->data[i]) ==
                        0) {
                        found = TRUE;
                        break;
                    }
                }

                if (!found) {
                    ENGINE_INFO("Required extension not found: '%s', skipping "
                                "device '%s'",
                                requirements->device_extension_names->data[i],
                                properties->deviceName);

                    return FALSE;
                }
            }
        }

        return TRUE;
    }

    return FALSE;
}

void vulkan_device_query_swapchain_capabilities(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    Vulkan_Swapchain_Support_Info* out_swapchain_info) {

    out_swapchain_info->formats_count = -1;
    out_swapchain_info->present_modes_count = -1;

    VK_ENSURE_SUCCESS(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, surface, &out_swapchain_info->capabilities));

    u32 format_count;
    VK_ENSURE_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &format_count, nullptr));

    if (format_count != 0) {
        out_swapchain_info->formats_count = format_count;

        out_swapchain_info->formats = static_cast<VkSurfaceFormatKHR*>(
            memory_allocate(sizeof(VkSurfaceFormatKHR) * format_count,
                            Memory_Tag::RENDERER));

        VK_ENSURE_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, surface, &format_count, out_swapchain_info->formats));
    }

    u32 present_modes_count;
    VK_ENSURE_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &present_modes_count, nullptr));

    if (present_modes_count != 0) {
        out_swapchain_info->present_modes_count = present_modes_count;

        out_swapchain_info->present_modes = static_cast<VkPresentModeKHR*>(
            memory_allocate(sizeof(VkPresentModeKHR) * present_modes_count,
                            Memory_Tag::RENDERER));

        VK_ENSURE_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &present_modes_count,
            out_swapchain_info->present_modes));
    }
}

void vulkan_device_shutdown(Vulkan_Context* context) {
    if (context->device.logical_device) {
        ENGINE_INFO("Destroying logical device recource...");

        vkDestroyDevice(context->device.logical_device, context->allocator);
        context->device.logical_device = nullptr;
    }

    if (context->device.swapchain_info.formats) {
        memory_deallocate(context->device.swapchain_info.formats,
                          sizeof(VkSurfaceFormatKHR) *
                              context->device.swapchain_info.formats_count,
                          Memory_Tag::RENDERER);

        context->device.swapchain_info.formats = nullptr;
        context->device.swapchain_info.formats_count = 0;
    }

    if (context->device.swapchain_info.present_modes) {
        memory_deallocate(
            context->device.swapchain_info.present_modes,
            sizeof(VkPresentModeKHR) *
                context->device.swapchain_info.present_modes_count,
            Memory_Tag::RENDERER);

        context->device.swapchain_info.present_modes = nullptr;
        context->device.swapchain_info.present_modes_count = 0;
    }

    context->device.presentation_queue = nullptr;
    context->device.graphics_queue = nullptr;
    context->device.transfer_queue = nullptr;

    // Since the physical device is not created, but just obtained, there is
    // nothing to free, exept the utilized resources
    ENGINE_INFO("Releasing physical device resource...");
    context->device.physical_device = nullptr;

    context->device.graphics_queue_index = -1;
    context->device.transfer_queue_index = -1;
    context->device.compute_queue_index = -1;
}
