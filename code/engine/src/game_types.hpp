#pragma once

#include "defines.hpp"
#include "core/application.hpp"

struct Game {
    Application_Config config;

    b8 (*initialize)(Game*);
    b8 (*update)(Game*, f32 delta_time);
    b8 (*render)(Game*, f32 delta_time);
    void (*on_resize)(Game*, u32 width, u32 height);
    void (*shutdown)(Game*); // Here only to deallocate the game_state once the application finishes

    void* state;
};
