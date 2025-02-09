#pragma once

#include "defines.hpp"
#include "core/logger.hpp"

extern b8 create_game();

int main() {
    if(!create_game()) {
        ENGINE_FATAL("Failed to create game application");
        return -1;
    }

    ENGINE_INFO("Game application created successfully");
    return 0;
}
