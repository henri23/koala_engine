#include "vulkan_device.hpp"
#include <vulkan/vulkan_core.h>

b8 vulkan_device_select(Vulkan_Context* context) {

    u32 physical_device_count = 0;

    // Retrieve the list of available GPUs with vulkan support
    vkEnumeratePhysicalDevices(context->instance,
                               &physical_device_count,
                               nullptr);

    // Check if there's at least one GPU that supports Vulkan
    if (physical_device_count == 0)
        return FALSE;

    VkPhysicalDevice physical_devices_array[physical_device_count];

    vkEnumeratePhysicalDevices(context->instance,
                               &physical_device_count,
                               physical_devices_array);

    // Evaluate GPUs -> If mutliple GPUs are present in the machine, we need to pick
    // the most "qualified" one
    for (u32 i = 0; i < physical_device_count; ++i) {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        VkPhysicalDeviceMemoryProperties device_memory_properties;

        vkGetPhysicalDeviceProperties(
            physical_devices_array[i],
            &device_properties);
        vkGetPhysicalDeviceFeatures(
            physical_devices_array[i],
            &device_features);
        vkGetPhysicalDeviceMemoryProperties(
            physical_devices_array[i],
            &device_memory_properties);

		// Score the GPUs based on the properties they provide
		
    }

    return TRUE;
}

void vulkan_device_shutdown(Vulkan_Context* context) {
}
