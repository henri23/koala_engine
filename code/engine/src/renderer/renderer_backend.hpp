#pragma once

#include "renderer/renderer_types.inl"

struct Platform_State;

b8 renderer_backend_initialize(
    Renderer_Backend_Type type,
    Renderer_Backend* out_backend);

void renderer_backend_shutdown(
    Renderer_Backend* backend);

