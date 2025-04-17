#include "vulkan_swapchain.hpp"

#include "core/logger.hpp"
#include "core/memory.hpp"

#include "vulkan_device.hpp"
#include <vulkan/vulkan_core.h>

b8 vulkan_swapchain_create(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    Vulkan_Swapchain* out_swapchain) {

    Vulkan_Swapchain_Support_Info* swapchain_info = &context->device.swapchain_info;

    VkSurfaceFormatKHR* selected_format;

    // Choose prefered format from available formats of the logical device
    // The formats and present modes array are setup on device selection in
    // vulkan_device_query_swapchain_capabilities()
    for (u32 i = 0; i < swapchain_info->formats_count; ++i) {
        if (swapchain_info->formats[i].format ==
                VK_FORMAT_B8G8R8_SRGB &&
            swapchain_info->formats[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selected_format = &swapchain_info->formats[i];
            break;
        }
    }

    // If the requested format was not found then pick the first one available
    if (!selected_format)
        selected_format =
            &swapchain_info->formats[0];

    VkPresentModeKHR selected_present_mode;

    // For present modes all GPU must implement VK_PRESENT_MODE_FIFO_KHR, and
    // it is the most similar to how OpenGl works, however my first choice
    // would be VK_PRESENT_MODE_MAILBOX_KHR
    for (u32 i = 0; i < swapchain_info->present_modes_count; ++i) {
        if (swapchain_info->present_modes[i] ==
            VK_PRESENT_MODE_MAILBOX_KHR)

            selected_present_mode =
                swapchain_info->present_modes[i];
        break;
    }

    if (!selected_present_mode)
        selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    // The swap extent is the resolution of the swap chain images and it is
    // almost always equal to the resolution of the windows (with the exception)
    // of Apple's Retina Displas (TODO).

    VkExtent2D actual_extent = {
        CLAMP(width,
              (u32)swapchain_info->capabilities.minImageExtent.width,
              (u32)swapchain_info->capabilities.maxImageExtent.width),

        CLAMP(height,
              (u32)swapchain_info->capabilities.minImageExtent.height,
              (u32)swapchain_info->capabilities.maxImageExtent.height),
    };

    u32 image_count = CLAMP(swapchain_info->capabilities.minImageCount + 1,
                            0,
                            swapchain_info->capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR create_info =
        {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};

    create_info.surface = context->surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = selected_format->format;
    create_info.imageColorSpace = selected_format->colorSpace;
    create_info.imageExtent = actual_extent;

    // imageArrayLayers specifies the amount of layers each image consists of.
    // This is always 1 unless we are developing a stereoscopic 3D application
    create_info.imageArrayLayers = 1;

    // If we want to use images just as a canvas where we draw we use the color
    // attachment bit as ImageUsage. It is also possible to first draw into
    // anther image and do some post-processing operation and later render the
    // image with VK_IMAGE_USAGE_TRANSFER_DST_BIT. This requires memory ops.
    // to transfer the rendered image to the swap chain
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Since the same swapchain will be used accross multiple queues, we need
    // to specify how the images that are accessed by multiple queues will be
    // handled.
    // - VK_SHARING_MODE_EXCLUSIVE: an image is owned by one queue family at a
    // time and ownership must be explicitly transferred before using it in
    // another queue family
    // - VK_SHARING_MODE_CONCURRENT: images can be used across multiple queues
    // without explicit ownership transfer
    u32 queue_family_indices[] = {
        context->device.graphics_queue_index,
        context->device.present_queue_index};

    if (context->device.graphics_queue_index !=
        context->device.present_queue_index) {

        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;     // optional
        create_info.pQueueFamilyIndices = nullptr; // optional
    }

    create_info.preTransform = swapchain_info->capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = selected_present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VK_ENSURE_SUCCESS(vkCreateSwapchainKHR(
        context->device.logical_device,
        &create_info,
        context->allocator,
        &context->swapchain.handle))

	ENGINE_INFO("Vulkan swapchain successfully created.");
    return TRUE;
}

void vulkan_swapchain_shutdown(
    Vulkan_Context* context,
    Vulkan_Swapchain* swapchain) {

	ENGINE_INFO("Destroying Vulkan swapchain...");
    vkDestroySwapchainKHR(
        context->device.logical_device,
        swapchain->handle,
        context->allocator);

	ENGINE_INFO("Swapchain destroyed.");
}
