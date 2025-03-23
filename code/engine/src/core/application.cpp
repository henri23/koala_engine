#include "application.hpp"

#include "core/logger.hpp"
#include "core/memory.hpp"
#include "platform/platform.hpp"
#include "containers/darray.hpp"

struct application_state {
    b8 is_running;
    b8 is_suspended;
    platform_state platform_state;
};

internal b8 is_initialized = FALSE;
internal application_state application_state;

b8 application_initialize() {
    // TODO: Automate process of susbsytem initialization in a queue with dependency order optimization

    // Protect the application state from being initialized multiple times
    if (is_initialized) {
        ENGINE_ERROR("Engine subsystems are already initialized");
        return FALSE;
    }

    // Initialize all subsystems
    log_startup();

    if (platform_startup(&application_state.platform_state, "Koala engine", 500, 500, 500, 500) == FALSE) { // Depends on: logger
        return FALSE;
    }

    memory_startup(); // Depends on: platform, logger

    ENGINE_DEBUG("Subsystems initialized correctly.");
    is_initialized = TRUE;

    ENGINE_DEBUG(memory_get_current_usage()); // WARN: Memory leak because the heap allocated string must be deallocated

    return TRUE;
}

void application_run() {
    application_state.is_running = TRUE;
    ENGINE_DEBUG("Application loop is starting...");

    while (application_state.is_running) {
        // For each iteration read the new messages from the message queue
        if (!platform_read_events(&application_state.platform_state)) {
            application_state.is_running = FALSE;
        }

        // Frame
    }

    application_shutdown();
}

void application_shutdown() {

    // Start shutting down subsystems in reverse order to the startup order
    memory_shutdown();
    platform_shutdown(&application_state.platform_state);
    log_shutdown();

    ENGINE_DEBUG("Application susbsytems stopped correctly");
}
