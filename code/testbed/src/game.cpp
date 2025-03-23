#include "game.hpp"

#include <core/logger.hpp>

b8 game_initialize(game* game_inst) {
    GAME_INFO("Called game_initialize()");
    return TRUE;
}

b8 game_update(game* game_inst, f32 delta_time) {
    // GAME_INFO("Called game_update()");
    return TRUE;
}

b8 game_render(game* game_inst, f32 delta_time) {
    // GAME_INFO("Called game_render()");
    return TRUE;
}

void game_on_resize(game* game_inst, u32 width, u32 height) {
    GAME_INFO("Called game_on_resize()");
}
