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

    void* test_array = darray_create(u64);
    u64 test1 = 4;
    u64 test2 = 5;
    u64 test3 = 6;
    u64 test4 = 7;
    darray_push(test_array, test1);
    darray_push(test_array, test2);
    darray_push(test_array, test3);
    darray_push(test_array, test4);
    u64 array_length = darray_length(test_array);
    for(u32 i = 0; i < array_length; ++i) {
        void* element = memory_allocate(sizeof(u64), MEMORY_TAG_UNKNOWN);
        darray_pop(test_array, element);
        ENGINE_DEBUG("Element %d:%d", i, *(static_cast<u64*>(element)));
        memory_deallocate(element, sizeof(u64), MEMORY_TAG_UNKNOWN);
    }
    darray_push(test_array, test1);
    darray_push(test_array, test2);
    darray_push(test_array, test3);
    darray_push(test_array, test4);
    darray_push_at(test_array, 2, test4); 
    array_length = darray_length(test_array);
    for(u32 i = 0; i < array_length; ++i) {
        void* element = memory_allocate(sizeof(u64), MEMORY_TAG_UNKNOWN);
        darray_pop(test_array, element);
        ENGINE_DEBUG("Element %d:%d", i, *(static_cast<u64*>(element)));
        memory_deallocate(element, sizeof(u64), MEMORY_TAG_UNKNOWN);
    }
    darray_destroy(test_array);

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
