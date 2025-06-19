#pragma once

#include "defines.hpp"
#include "core/application.hpp"
#include "core/logger.hpp"
#include "game_types.hpp"

extern b8 create_game(Game* game_inst);

int main() {

    Game game_inst = {};
    if(!create_game(&game_inst)) {
        ENGINE_FATAL("Failed to create game");
        return -1;
    }

    if(!game_inst.initialize || !game_inst.update || !game_inst.render || !game_inst.on_resize){
        ENGINE_FATAL("Missing game callback functions");
        return -1; 
    }

    if(!application_initialize(&game_inst)) {
        ENGINE_FATAL("Failed to create application");
        return -1;
    }

    ENGINE_INFO("Game application created successfully");

    application_run();

    return 0;
}
