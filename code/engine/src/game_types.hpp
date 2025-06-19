#pragma once

#include "defines.hpp"
#include "core/application.hpp"

struct Game {
    Application_Config config;

    b8 (*initialize)(Game*);
    b8 (*update)(Game*, f32 delta_time);
    b8 (*render)(Game*, f32 delta_time);
    void (*on_resize)(Game*, u32 width, u32 height);
    void (*shutdown)(Game*); // Here only to deacmake testsllocate the game_state once the application finishes

    void* state; // Game-specific game state
	
	// It makes sense to store the application state on the game side, even
	// though it is an internal state, however since both the game state and the
	// application state have the same life-cycle (inspired from the Casey talk)
	// it makes sense for both these states to be stored and freed at the same
	// instants
	void* application_state; // Internal state of the engine application
};
