#pragma once

#include "renderer/renderer_types.inl"

struct Static_Mesh_Data;
struct Platform_State;

b8 renderer_startup(
    const char* application_name,
    struct Platform_State* plat_state);

void renderer_shutdown();

void renderer_on_resize(
    u16 width,
    u16 height);

b8 renderer_begin_frame(f32 delta_t);

b8 renderer_end_frame(f32 delta_t);

b8 renderer_draw_frame(Render_Packet* packet);
