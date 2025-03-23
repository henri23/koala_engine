#include <entry.hpp>
#include <core/memory.hpp>

#include "game.hpp"

b8 create_game(game* game_inst) {

    GAME_INFO("Called create_game()");

    // Add application configurations
    game_inst->config.name = "Koala engine";
    game_inst->config.start_pos_x = 500;
    game_inst->config.start_pos_y = 500;
    game_inst->config.start_width = 1280;
    game_inst->config.start_height = 720;
    game_inst->initialize = game_initialize;
    game_inst->render = game_render;
    game_inst->update = game_update;
    game_inst->on_resize = game_on_resize;
    game_inst->shutdown = game_shutdown;

    // WARN: Memory leak in this case because we never deallocate
    game_inst->state = memory_allocate(sizeof(game_state), memory_tag::GAME);
    return TRUE;
}
