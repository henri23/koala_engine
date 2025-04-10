#include "renderer/renderer_backend.hpp"

#include "vulkan/vulkan_backend.hpp"

b8 renderer_backend_initialize(
    Renderer_Backend_Type type,
    struct Platform_State* plat_state,
    Renderer_Backend* out_backend) {

    out_backend->plat_state = plat_state;

    // Instantiate specific backend
    switch (type) {
    case Renderer_Backend_Type::VULKAN: {
        out_backend->initialize = vulkan_initialize;
        out_backend->shutdown = vulkan_shutdown;
        out_backend->resized = vulkan_on_resized;
        out_backend->begin_frame = vulkan_begin_frame;
        out_backend->end_frame = vulkan_end_frame;

        return TRUE;
    }
    case Renderer_Backend_Type::OPENGL:
    case Renderer_Backend_Type::DIRECTX:
        break;
    }

    return FALSE;
}

void renderer_backend_shutdown(Renderer_Backend* backend) {
    backend->initialize = nullptr;
    backend->shutdown = nullptr;
    backend->resized = nullptr;
    backend->begin_frame = nullptr;
    backend->end_frame = nullptr;
}
