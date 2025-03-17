#pragma once

#include "defines.hpp"

struct platform_state {
    void* internal_state;
};

b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    s32 x,
    s32 y,
    s32 width,
    s32 height);
