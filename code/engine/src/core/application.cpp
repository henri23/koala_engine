#include "application.hpp"

#include "logger.hpp"
#include "platform/platform.hpp"

struct application_state {
    b8 is_running;
    b8 is_suspended;
    platform_state platform_state;
};

internal application_state application_state;

b8 application_initialize() {
    // Initialize all subsystems
    initialize_logging();

    if (platform_startup(&application_state.platform_state, "Koala engine", 500, 500, 500, 500) == FALSE) {
        return FALSE;
    }

    ENGINE_DEBUG("Subsystems initialized correctly.");

    return TRUE;
}

void application_run() {
    application_state.is_running = TRUE;
    ENGINE_DEBUG("Application loop is starting...");

    while (application_state.is_running) {
        // For each iteration read the new messages from the message queue 
        if(!platform_read_events(&application_state.platform_state)){
            application_state.is_running = FALSE;
        }

        // Frame 
    }

    application_shutdown();
}

void application_shutdown() {
    ENGINE_DEBUG("Application is being shut down...");
    platform_shutdown(&application_state.platform_state);
}
