#include "vulkan_swapchain.hpp"

#include "core/logger.hpp"
#include "core/memory.hpp"

#include "defines.hpp"
#include "renderer/vulkan/vulkan_device.hpp"
#include "renderer/vulkan/vulkan_types.hpp"
#include <vulkan/vulkan_core.h>

void create_swapchain(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    Vulkan_Swapchain* out_swapchain) {

    // To create the swapchain we need first to make 3 decisions:
    // 1. Choose the color format that we want
    // 2. Choose the present mode that we want
    // 3. Specify the extent of the image (size). This will be immutable so if
    // 	  the screen gets resized, the swapchain must be recreated with the new
    // 	  size

    Vulkan_Swapchain_Support_Info* swapchain_info = &context->device.swapchain_info;

    // Choose prefered format from available formats of the logical device
    // The formats and present modes array are setup on device selection in
    // vulkan_device_query_swapchain_capabilities()

    b8 found = FALSE;
    for (u32 i = 0; i < swapchain_info->formats_count; ++i) {
        if (swapchain_info->formats[i].format ==
                VK_FORMAT_B8G8R8_SRGB &&
            swapchain_info->formats[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            context->swapchain.image_format = swapchain_info->formats[i];
            found = TRUE;
            break;
        }
    }

    // If the requested format was not found then pick the first one available
    if (!found)
        context->swapchain.image_format = swapchain_info->formats[0];

    vulkan_device_query_swapchain_capabilities(
        context->device.physical_device,
        context->surface,
        &context->device.swapchain_info);

    // For present modes all GPU must implement VK_PRESENT_MODE_FIFO_KHR, and
    // it is the most similar to how OpenGl works, however my first choice
    // would be VK_PRESENT_MODE_MAILBOX_KHR
    VkPresentModeKHR selected_present_mode;
    found = FALSE;

    for (u32 i = 0; i < swapchain_info->present_modes_count; ++i) {
        if (swapchain_info->present_modes[i] ==
            VK_PRESENT_MODE_MAILBOX_KHR) {

            selected_present_mode = swapchain_info->present_modes[i];
            found = TRUE;
            break;
        }
    }

    if (!found)
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

    VkSwapchainCreateInfoKHR create_info =
        {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};

    create_info.surface = context->surface;

    // Set the minimum image count in the swapchain, but nothing forbids the
    // swapchain to have more images than this
    u32 image_count = CLAMP(swapchain_info->capabilities.minImageCount + 1,
                            0,
                            swapchain_info->capabilities.maxImageCount);
    context->swapchain.max_frames_in_process = 2;

    create_info.minImageCount = image_count;

    create_info.imageFormat = context->swapchain.image_format.format;
    create_info.imageColorSpace = context->swapchain.image_format.colorSpace;
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

    // In theory Vulkan can destroy the an existing swapchain by passing
    // the reference here, however it is better to explicitly destroy
    // any older swapchain
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VK_ENSURE_SUCCESS(vkCreateSwapchainKHR(
        context->device.logical_device,
        &create_info,
        context->allocator,
        &context->swapchain.handle))

    ENGINE_INFO("Vulkan swapchain successfully created.");

    context->swapchain.extent = actual_extent;

    context->current_frame = 0;
    context->swapchain.image_count = 0;

    vkGetSwapchainImagesKHR(
        context->device.logical_device,
        context->swapchain.handle,
        &context->swapchain.image_count,
        nullptr);

    if (!context->swapchain.images) {
        context->swapchain.images = static_cast<VkImage*>(
            memory_allocate(
                sizeof(VkImage) * context->swapchain.image_count,
                Memory_Tag::RENDERER));
    }

    if (!context->swapchain.views) {
        context->swapchain.views = static_cast<VkImageView*>(
            memory_allocate(
                sizeof(VkImageView) * context->swapchain.image_count,
                Memory_Tag::RENDERER));
    }

    vkGetSwapchainImagesKHR(
        context->device.logical_device,
        context->swapchain.handle,
        &context->swapchain.image_count,
        context->swapchain.images);

    for (u32 i = 0; i < context->swapchain.image_count; ++i) {
        VkImageViewCreateInfo view_info =
            {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

        view_info.image = context->swapchain.images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = context->swapchain.image_format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // The components field allows us to swizzle the color channels. If
        // we map all the channels to the red channel, we will get a monochrome
        // texture. I am keeping the default mapping here
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // The subresourceRange field describes what the image's purpose is and
        // which part of the image should be accessed. The images in this engine
        // will be color targets without any mipmapping levels or multiple layers
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        VK_ENSURE_SUCCESS(
            vkCreateImageView(
                context->device.logical_device,
                &view_info,
                context->allocator,
                &context->swapchain.views[i]));
    }

    // Create depth buffer. The depth buffer is an image cotaining the depth
    // from the camera point of view
    if (!vulkan_device_detect_depth_format(&context->device)) {
        context->device.depth_format = VK_FORMAT_UNDEFINED;
        ENGINE_FATAL("Failed to find a supported depth format!");
    }
}

void vulkan_swapchain_create(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    Vulkan_Swapchain* out_swapchain) {

    create_swapchain(
        context,
        width,
        height,
        out_swapchain);
}

void vulkan_swapchain_recreate(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    Vulkan_Swapchain* out_swapchain) {

    ENGINE_DEBUG("Destroying previous swapchain...");

    vulkan_swapchain_destroy(
        context,
        out_swapchain);

    ENGINE_DEBUG(
        "Recreating swapchain with sizes { %d ; %d }",
        width,
        height);

    create_swapchain(
        context,
        width,
        height,
        out_swapchain);
}

void vulkan_swapchain_present(
    Vulkan_Context* context,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index) {

    VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &context->swapchain.handle;
    present_info.pImageIndices = &present_image_index;
    present_info.pResults = 0;

    VkResult result = vkQueuePresentKHR(present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        vulkan_swapchain_recreate(
            context,
            context->framebuffer_width,
            context->framebuffer_height,
            &context->swapchain);

    } else if (result != VK_SUCCESS) {
        ENGINE_FATAL("Failed to present swap chain image!");
    }
}

b8 vulkan_swapchain_get_next_image_index(
    Vulkan_Context* context,
    u64 timeout_ns,
    VkSemaphore image_available_semaphore,
    VkFence fence,
    u32* out_image_index) {

    VkResult result = vkAcquireNextImageKHR(
        context->device.logical_device,
        context->swapchain.handle,
        timeout_ns,
        image_available_semaphore,
        fence,
        out_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vulkan_swapchain_recreate(
            context,
            context->framebuffer_width,
            context->framebuffer_height,
            &context->swapchain);

        return FALSE;

    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        ENGINE_FATAL("Failed to acquire swapchain iamge!");
        return FALSE;
    }

    return TRUE;
}

void vulkan_swapchain_destroy(
    Vulkan_Context* context,
    Vulkan_Swapchain* swapchain) {

    ENGINE_DEBUG("Destroying image views... Found %d views",
                 context->swapchain.image_count);

    for (u32 i = 0; i < context->swapchain.image_count; ++i) {
        vkDestroyImageView(
            context->device.logical_device,
            context->swapchain.views[i],
            context->allocator);
    }

    ENGINE_DEBUG("All image views destroyed");

    ENGINE_INFO("Destroying Vulkan swapchain...");

    memory_deallocate(
        context->swapchain.views,
        sizeof(VkImageView) * context->swapchain.image_count,
        Memory_Tag::RENDERER);

    memory_deallocate(
        context->swapchain.images,
        sizeof(VkImage) * context->swapchain.image_count,
        Memory_Tag::RENDERER);

    vkDestroySwapchainKHR(
        context->device.logical_device,
        swapchain->handle,
        context->allocator);

    ENGINE_INFO("Swapchain destroyed.");
}
