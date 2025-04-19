#pragma once

#include "vulkan_types.hpp"

void vulkan_swapchain_create(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    Vulkan_Swapchain* out_swapchain);

void vulkan_swapchain_recreate(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    Vulkan_Swapchain* out_swapchain);

void vulkan_swapchain_destroy(
    Vulkan_Context* context,
    Vulkan_Swapchain* swapchain);

b8 vulkan_swapchain_get_next_image_index(
    Vulkan_Context* context,
    u64 timeout_ns,
    VkSemaphore image_available_semaphore,
    VkFence fence,
    u32* out_image_index);

void vulkan_swapchain_present(
    Vulkan_Context* context,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index);
