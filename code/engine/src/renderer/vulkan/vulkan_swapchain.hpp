#pragma once

#include "vulkan_types.hpp"

b8 vulkan_swapchain_create(
    Vulkan_Context* context,
	u32 width,
	u32 height,
    Vulkan_Swapchain* out_swapchain);

void vulkan_swapchain_shutdown(
	Vulkan_Context* context, 
	Vulkan_Swapchain* swapchain);
