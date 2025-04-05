#pragma once

#include "defines.hpp"

enum class Renderer_Backend_Type {
    VULKAN,
    OPENGL,
    DIRECTX
};

struct Renderer_Backend {
    struct Platform_State* plat_state;

    b8 (*initialize)(
        Renderer_Backend* backend,
        const char* app_name,
        struct Platform_State* plat_state);

    void (*shutdown)(
        Renderer_Backend* backend);

    void (*resized)(
        Renderer_Backend* backend,
        u16 width,
        u16 height);

    b8 (*begin_frame)(
        Renderer_Backend* backend,
        f32 delta_t);

    b8 (*end_frame)(
        Renderer_Backend* backend,
        f32 delta_t);
};
