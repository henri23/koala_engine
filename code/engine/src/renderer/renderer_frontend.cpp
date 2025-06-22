#include "renderer/renderer_frontend.hpp"
#include "renderer/renderer_backend.hpp"

#include "core/logger.hpp"
#include "core/memory.hpp"

// Renderer frontend will manage the backend interface state
struct Renderer_System_State {
    Renderer_Backend backend;
};

internal Renderer_System_State* state_ptr;

b8 renderer_startup(
    u64* memory_requirements,
    void* state,
    const char* application_name) {

    *memory_requirements = sizeof(Renderer_System_State);

    if (state == nullptr) {
        return TRUE;
    }

    state_ptr = static_cast<Renderer_System_State*>(state);

    if (!renderer_backend_initialize(
            Renderer_Backend_Type::VULKAN,
            &state_ptr->backend)) {

        ENGINE_INFO("Failed to initialize renderer backend");
        return FALSE;
    }

    state_ptr->backend.initialize(
        &state_ptr->backend,
        application_name);

    ENGINE_DEBUG("Renderer subsystem initialized");
    return TRUE;
}

void renderer_shutdown(void* state) {
    state_ptr->backend.shutdown(&state_ptr->backend);

    renderer_backend_shutdown(&state_ptr->backend);

	state_ptr = nullptr;

    ENGINE_DEBUG("Renderer subsystem shutting down...");
}

void renderer_on_resize(
    u16 width,
    u16 height) {

    state_ptr->backend.resized(
        &state_ptr->backend,
        width,
        height);
}

b8 renderer_begin_frame(f32 delta_t) {
    return state_ptr->backend.begin_frame(
        &state_ptr->backend,
        delta_t);
}

b8 renderer_end_frame(f32 delta_t) {
    b8 result = state_ptr->backend.end_frame(
        &state_ptr->backend,
        delta_t);

    state_ptr->backend.frame_number++;
    return result;
}

b8 renderer_draw_frame(Render_Packet* packet) {
    if (renderer_begin_frame(packet->delta_time)) {

        b8 result = renderer_end_frame(packet->delta_time);

        if (!result) {
            ENGINE_ERROR("renderer_end_frame failed. Application shutting down...");
            return FALSE;
        }
    }

    return TRUE;
}
