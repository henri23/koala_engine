#pragma once

#include "vulkan_types.hpp"

b8 vulkan_swapchain_create(
    Vulkan_Context* context,
    VkSwapchainKHR* out_swapchain);

b8 vulkan_swapchain_shutdown();
