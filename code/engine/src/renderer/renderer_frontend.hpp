#pragma once

#include "renderer/renderer_types.hpp"

struct Static_Mesh_Data;
struct Platform_State;

b8 renderer_startup(
    const char* application_name,
    struct Platform_State* plat_state);

void renderer_shutdown();

void renderer_on_resize(
    u16 width,
    u16 height);

b8 renderer_draw_frame(Render_Packet* packet);
