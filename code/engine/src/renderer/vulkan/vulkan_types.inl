#pragma once

#include "assert.h"
#include "containers/auto_array.hpp"
#include "defines.hpp"
#include <vulkan/vulkan.h>

#define VK_ENSURE_SUCCESS(expr) RUNTIME_ASSERT(expr == VK_SUCCESS);

struct Vulkan_Physical_Device_Requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    b8 discrete_gpu;
    b8 sampler_anisotropy;
    Auto_Array<const char*>* device_extension_names;
};

struct Vulkan_Device {
    VkPhysicalDevice physical_device; // Handle to physical device
    VkDevice logical_device;
	u32 grahic_queue_index;
	u32 transfer_queue_index;
	u32 compute_queue_index;
	u32 present_queue_index;
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;
	VkPhysicalDeviceMemoryProperties physical_device_memory;
};

struct Vulkan_Context {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkPhysicalDevice physical_device; // Implicitly destroyed destroying VkInstance

#ifdef DEBUG_BUILD
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    Vulkan_Device device;
};
