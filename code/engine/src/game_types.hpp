#pragma once

#include "defines.hpp"
#include "core/application.hpp"

struct game {
    application_config config;

    b8 (*initialize)(game* game_inst);
    b8 (*update)(game* game_inst, f32 delta_time);
    b8 (*render)(game* game_inst, f32 delta_time);
    void (*on_resize)(game* game_inst, u32 width, u32 height);

    void* state;
};
