#include "renderer/renderer_frontend.hpp"
#include "renderer/renderer_backend.hpp"

#include "core/logger.hpp"
#include "core/memory.hpp"

// Renderer frontend will manage the backend interface state
internal Renderer_Backend* backend;

b8 renderer_startup(
    const char* application_name,
    struct Platform_State* plat_state) {

    backend = static_cast<Renderer_Backend*>(
        memory_allocate(
            sizeof(Renderer_Backend),
            Memory_Tag::RENDERER));

    if (!renderer_backend_initialize(
            Renderer_Backend_Type::VULKAN,
            plat_state,
            backend)) {

        ENGINE_INFO("Failed to initialize renderer backend");
        return FALSE;
    }

    backend->initialize(
        backend,
        application_name,
        plat_state);

    ENGINE_DEBUG("Renderer subsystem initialized");
    return TRUE;
}

void renderer_shutdown() {
    backend->shutdown(backend);

	renderer_backend_shutdown(backend);

    memory_deallocate(
        backend,
        sizeof(Renderer_Backend),
        Memory_Tag::RENDERER);


    ENGINE_DEBUG("Renderer subsystem shutting down...");
}

void renderer_on_resize(
    u16 width,
    u16 height) {

    backend->resized(
        backend,
        width,
        height);
}

b8 renderer_draw_frame(Render_Packet* packet) {
    if (backend->begin_frame(
            backend,
            packet->delta_time)) {

        b8 result = backend->end_frame(
            backend,
            packet->delta_time);

        if (!result) {
            ENGINE_ERROR("renderer_end_frame failed. Application shutting down...");
            return FALSE;
        }
    }

    return TRUE;
}
