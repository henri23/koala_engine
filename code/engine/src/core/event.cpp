#include "core/event.hpp"

#include "containers/darray.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

struct registered_event {
    void* listener;
    PFN_event_handler callback;
};

// Entries must be dynamically allocated since we cannot predict how many listener there will be for each code
// We can use dynamic arrays to manage the elements
struct event_code_entry {
    registered_event* events;
};

#define MAX_EVENT_ENTRIES 16384

struct event_system_state {
    event_code_entry entries[MAX_EVENT_ENTRIES];
};

internal event_system_state state;
internal b8 is_initialized = FALSE;

b8 event_startup() {
    if (is_initialized == TRUE) {
        ENGINE_WARN("Event subsystem is already initialized");
        return FALSE;
    }

    memory_zero(&state, sizeof(state));

    ENGINE_DEBUG("Event subsystem initalized");

    is_initialized = TRUE;

    return TRUE;
}

void event_shutdown() {
    for (u32 i = 0; i < MAX_EVENT_ENTRIES; ++i)
        if (state.entries[i].events) {
            darray_destroy(state.entries[i].events);
            state.entries[i].events = nullptr;
        }

    ENGINE_DEBUG("Event subsystem shutting down...");
    is_initialized = FALSE;
}

b8 event_register_listener(
    event_code code,
    void* listener,
    PFN_event_handler on_event) {
    if (is_initialized == FALSE)
        return FALSE;

    // Check if array is initiliazed
    if (!state.entries[(u16)code].events)
        state.entries[(u16)code].events = static_cast<registered_event*>(
            darray_create(registered_event));

    void* events = state.entries[(u16)code].events;

    // Check if listener is already present
    u32 length = darray_length(events);
    for (u32 i = 0; i < length; ++i) {
        if (state.entries[(u16)code].events[i].listener == listener) {
            ENGINE_WARN("Listener for code is already registered");
            return FALSE;
        }
    }

    // No need to allocate in heap since the darray_push copies the entry
    registered_event entry;

    entry.listener = listener;
    entry.callback = on_event;

    darray_push(events, entry);

    // Since the address of the pointer could change in the push we need to reasign the pointer
    state.entries[(u16)code].events = static_cast<registered_event*>(events);

    return TRUE;
}

b8 event_unregister_listener(
    event_code code,
    void* listener,
    PFN_event_handler on_event) {
    if (is_initialized == FALSE) {
        return FALSE;
    }

    // Check if array is initiliazed
    if (!state.entries[(u16)code].events)
        return FALSE;

    // Check if listener is already present
    u32 length = darray_length(
        static_cast<void*>(
            state.entries[(u16)code].events));

    for (u32 i = 0; i < length; ++i) {
        registered_event e = state.entries[(u16)code].events[i];

        if (
            e.listener == listener &&
            e.callback == on_event) {
            registered_event popped;

            darray_pop_at(
                static_cast<void*>(
                    state.entries[(u16)code].events),
                i,
                &popped);

            return TRUE;
        }
    }

    ENGINE_WARN("Listener not found");
    return FALSE;
}

b8 event_fire(
    event_code code,
    void* sender,
    event_context context) {
    if (is_initialized == FALSE) {
        return FALSE;
    }

    // Check if array is initiliazed
    if (!state.entries[(u16)code].events)
        return FALSE;

    // Check if listener is already present
    u32 length = darray_length((void*)(state.entries[(u16)code].events));
    for (u32 i = 0; i < length; ++i) {
        registered_event e = state.entries[(u16)code].events[i];
        if (e.callback(
                code,
                sender,
                e.listener,
                context)) {
            // If a handler returns TRUE the event will not be consumed by the remaining listeners
            return TRUE;
        }
    }

    if (length == 0)
        ENGINE_WARN("No listener found for event");

    return FALSE;
}
