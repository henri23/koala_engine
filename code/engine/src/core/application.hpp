#pragma once

#include "defines.hpp"

struct game; // Forward declare game struct since this file needs to be included inside the game header

struct application_config {
    s16 start_pos_x;
    s16 start_pos_y;
    s16 start_height;
    s16 start_width;

    const char* name;
};

b8 application_initialize(game* game_inst);

void application_run();

void application_shutdown();

