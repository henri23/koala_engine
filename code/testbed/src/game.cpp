#include "game.hpp"
#include <core/memory.hpp>
#include <core/logger.hpp>

b8 game_initialize(Game* game_inst) {
    GAME_INFO("Called game_initialize()");
    return TRUE;
}

b8 game_update(Game* game_inst, f32 delta_time) {
    // GAME_INFO("Called game_update() with delta_t %.4f ms", delta_time * 1000);
    return TRUE;
}

b8 game_render(Game* game_inst, f32 delta_time) {
    // GAME_INFO("Called game_render()");
    return TRUE;
}

void game_on_resize(Game* game_inst, u32 width, u32 height) {
    GAME_INFO("Called game_on_resize() with Width: %d and Height: %d", width, height);
}

void game_shutdown(Game* game_inst) {
    GAME_INFO("Called game_shutdown()");
    memory_deallocate(game_inst->state, sizeof(Game_State), Memory_Tag::GAME);
}
