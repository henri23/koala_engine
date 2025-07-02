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

internal Event_System_State* state_ptr = nullptr;

b8 event_startup(u64* mem_req, void* state) {

    *mem_req = sizeof(Event_System_State);

    if (state == nullptr) {
        return true;
    }

    state_ptr = static_cast<Event_System_State*>(state);

    memory_zero(state_ptr, sizeof(Event_System_State));

    ENGINE_DEBUG("Event subsystem initalized");

    return true;
}

void event_shutdown(void* state) {
    for (u32 i = 0; i < MAX_EVENT_ENTRIES; ++i)
        if (state_ptr->entries[i].event_listeners.data) {
            state_ptr->entries[i].event_listeners.free();
        }

	state_ptr = nullptr;

    ENGINE_DEBUG("Event subsystem shutting down...");
}

b8 event_register_listener(
    Event_Code code,
    void* listener,
    PFN_Event_Handler on_event) {

    Auto_Array<Registered_Event>* events_array =
        &state_ptr->entries[(u16)code].event_listeners;

    // Check if listener is already present
    for (u32 i = 0; i < events_array->length; ++i) {
        if ((*events_array)[i].listener == listener) {
            ENGINE_WARN("Listener for code is already registered");
            return false;
        }
    }

    // No need to allocate in heap since the darray_push copies the entry
    Registered_Event entry;

    entry.listener = listener;
    entry.callback = on_event;

    events_array->add(entry);

    return true;
}

b8 event_unregister_listener(
    Event_Code code,
    void* listener,
    PFN_Event_Handler on_event) {

    // Check if array is initiliazed
    if (!state_ptr->entries[(u16)code].event_listeners.data)
        return false;

    Auto_Array<Registered_Event>* events_array =
        &state_ptr->entries[(u16)code].event_listeners;

    for (u32 i = 0; i < events_array->length; ++i) {
        Registered_Event e = (*events_array)[i];

        if (
            e.listener == listener &&
            e.callback == on_event) {

            events_array->pop_at(i);

            return true;
        }
    }

    ENGINE_WARN("Listener not found");
    return false;
}

b8 event_fire(
    Event_Code code,
    void* sender,
    Event_Context context) {

    // Check if array is initiliazed
    if (!state_ptr->entries[(u16)code].event_listeners.data)
        return false;

    Auto_Array<Registered_Event>* events = &state_ptr->entries[(u16)code].event_listeners;

    // Check if listener is already present
    for (u32 i = 0; i < events->length; ++i) {
        Registered_Event e = (*events)[i];
        if (e.callback(
                code,
                sender,
                e.listener,
                context)) {
            // If a handler returns true the event will not be consumed by the remaining listeners
            return true;
        }
    }

    if (events->length == 0)
        ENGINE_WARN("No listener found for event");

    return false;
}
