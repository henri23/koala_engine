#include "application.hpp"
#include "logger.hpp"

struct application_state {
    b8 is_running;
    b8 is_suspended;
};

internal application_state application_state;

b8 create_application() {
    // Initialize all subsystems
    initialize_logging();

    ENGINE_DEBUG("Subsystems initialized correctly.");

    return TRUE;
}

void run_application() {
    application_state.is_running = TRUE;
    ENGINE_DEBUG("Application loop is starting...");

    while(application_state.is_running) {
        
    }
}
