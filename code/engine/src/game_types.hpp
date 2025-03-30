#pragma once

#include "defines.hpp"
#include "core/application.hpp"

struct game {
    application_config config;

    b8 (*initialize)(game*);
    b8 (*update)(game*, f32 delta_time);
    b8 (*render)(game*, f32 delta_time);
    void (*on_resize)(game*, u32 width, u32 height);
    void (*shutdown)(game*); // Here only to deallocate the game_state once the application finishes

    void* state;
};
