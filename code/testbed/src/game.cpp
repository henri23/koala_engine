#include "game.hpp"
#include <core/input.hpp>
#include <core/logger.hpp>
#include <core/memory.hpp>

b8 game_initialize(Game* game_inst) {
    GAME_INFO("Called game_initialize()");
    return true;
}

b8 game_update(Game* game_inst, f32 delta_time) {
    // GAME_INFO("Called game_update() with delta_t %.4f ms", delta_time * 1000);
    local_persist u64 alloc_count = 0;
    u64 current_alloc_count = alloc_count;
    alloc_count = memory_get_allocations_count();

    if (input_is_key_up(Keyboard_Key::M) &&
        input_was_key_down(Keyboard_Key::M)) {

        GAME_DEBUG(
            "Allocations: %llu (%llu this frame)",
            alloc_count,
            alloc_count - current_alloc_count);
    }

    return true;
}

b8 game_render(Game* game_inst, f32 delta_time) {
    // GAME_INFO("Called game_render()");
    return true;
}

void game_on_resize(Game* game_inst, u32 width, u32 height) {
    GAME_INFO("Called game_on_resize() with Width: %d and Height: %d", width, height);
}

void game_shutdown(Game* game_inst) {
    GAME_INFO("Called game_shutdown()");
    memory_deallocate(game_inst->state, sizeof(Game_State), Memory_Tag::GAME);
}
