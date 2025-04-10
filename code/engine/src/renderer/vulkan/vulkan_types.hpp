#pragma once

#include "defines.hpp"
#include "assert.h"
#include <vulkan/vulkan.h>

#define VK_ENSURE_SUCCESS(expr) RUNTIME_ASSERT(expr == VK_SUCCESS);

struct Vulkan_Context{
	VkInstance instance;
	VkAllocationCallbacks* allocator;
#ifdef DEBUG_BUILD
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};
