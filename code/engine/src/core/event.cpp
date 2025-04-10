#include "core/event.hpp"

#include "containers/auto_array.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

struct Registered_Event {
    void* listener;
    PFN_Event_Handler callback;
};

// Entries must be dynamically allocated since we cannot predict how many listener there will be for each code
// We can use dynamic arrays to manage the elements
struct Event_Code_Entry {
    Auto_Array<Registered_Event> event_listeners;
};

#define MAX_EVENT_ENTRIES 16384

struct Event_System_State {
    Event_Code_Entry entries[MAX_EVENT_ENTRIES];
};

internal Event_System_State state;
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
        if (state.entries[i].event_listeners.data) {
            state.entries[i].event_listeners.free();
        }

    ENGINE_DEBUG("Event subsystem shutting down...");
    is_initialized = FALSE;
}

b8 event_register_listener(
    Event_Code code,
    void* listener,
    PFN_Event_Handler on_event) {
    if (is_initialized == FALSE)
        return FALSE;

    Auto_Array<Registered_Event>* events_array = 
		&state.entries[(u16)code].event_listeners;

    // Check if listener is already present
    for (u32 i = 0; i < events_array->length; ++i) {
        if ((*events_array)[i].listener == listener) {
            ENGINE_WARN("Listener for code is already registered");
            return FALSE;
        }
    }

    // No need to allocate in heap since the darray_push copies the entry
    Registered_Event entry;

    entry.listener = listener;
    entry.callback = on_event;

    events_array->add(entry);

    return TRUE;
}

b8 event_unregister_listener(
    Event_Code code,
    void* listener,
    PFN_Event_Handler on_event) {
    if (is_initialized == FALSE) {
        return FALSE;
    }

    // Check if array is initiliazed
    if (!state.entries[(u16)code].event_listeners.data)
        return FALSE;

    Auto_Array<Registered_Event>* events_array = 
		&state.entries[(u16)code].event_listeners;

    for (u32 i = 0; i < events_array->length; ++i) {
        Registered_Event e = (*events_array)[i];

        if (
            e.listener == listener &&
            e.callback == on_event) {

			events_array->pop_at(i);

            return TRUE;
        }
    }

    ENGINE_WARN("Listener not found");
    return FALSE;
}

b8 event_fire(
    Event_Code code,
    void* sender,
    Event_Context context) {
    if (is_initialized == FALSE) {
        return FALSE;
    }

    // Check if array is initiliazed
    if (!state.entries[(u16)code].event_listeners.data)
        return FALSE;

    Auto_Array<Registered_Event>* events = &state.entries[(u16)code].event_listeners;

    // Check if listener is already present
    for (u32 i = 0; i < events->length; ++i) {
        Registered_Event e = (*events)[i];
        if (e.callback(
                code,
                sender,
                e.listener,
                context)) {
            // If a handler returns TRUE the event will not be consumed by the remaining listeners
            return TRUE;
        }
    }

    if (events->length == 0)
        ENGINE_WARN("No listener found for event");

    return FALSE;
}
