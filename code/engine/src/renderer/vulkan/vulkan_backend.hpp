#pragma once

#include "renderer/renderer_types.hpp"

b8 vulkan_initialize(
    Renderer_Backend* backend,
    const char* app_name,
    struct Platform_State* plat_state);

void vulkan_shutdown(
    Renderer_Backend* backend);

void vulkan_on_resized(
    Renderer_Backend* backend,
    u16 width,
    u16 height);

b8 vulkan_begin_frame(
    Renderer_Backend* backend,

    f32 delta_t);

b8 vulkan_end_frame(
    Renderer_Backend* backend,
    f32 delta_t);
