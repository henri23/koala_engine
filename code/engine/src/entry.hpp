#pragma once

#include "defines.hpp"
#include "core/application.hpp"
#include "core/logger.hpp"

extern b8 create_game();

int main(void) {
    if(!create_game()) {
        ENGINE_FATAL("Failed to create game");
        return -1;
    }

    if(!create_application()) {
        ENGINE_FATAL("Failed to create application");
        return -1;
    }

    ENGINE_INFO("Game application created successfully");

    run_application();

    return 0;
}
