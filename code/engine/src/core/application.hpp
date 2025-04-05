#pragma once

#include "defines.hpp"

struct Game; // Forward declare game struct since this file needs to be included inside the game header

struct Application_Config {
    s16 start_pos_x;
    s16 start_pos_y;
    s16 start_height;
    s16 start_width;

    b8 limit_frame;
    const char* name;
};

b8 application_initialize(Game* game_inst);

void application_run();

void application_shutdown();

