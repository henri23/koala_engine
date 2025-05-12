#pragma once

#include "defines.hpp"

// NOTE: The event context size will be 16 bytes
struct Event_Context {
    union {
        u64 u64[2];
        s64 s64[2];
        f64 f64[2];

        u32 u32[4];
        s32 s32[4];
        f32 f32[4];

        u16 u16[8];
        s16 s16[8];

        u8 u8[16];
        s8 s8[16];

        char c[16];
    } data;
};

// NOTE: User application codes should start beyond 255
enum class Event_Code : u16 {
    APPLICATION_QUIT = 0x01,

    // Key code will be contained in u16[0] since keycodes are expressed in u16
    // Key modifies will be contained in u16[1] and will be a masked value like the xcb specifies
    KEY_PRESSED = 0x02,
    KEY_RELEASED = 0x03,

    BUTTON_PRESSED = 0x04,
    BUTTON_RELEASED = 0x05,

    // Mouse coordinate will be in the u16[0] = x and u16[1] = y
    MOUSE_MOVED = 0x06,
    MOUSE_WHEEL = 0x07,

    RESIZED = 0x08,

    MAX_EVENT_CODE = 0xFF
};

// When an event is processed in the bus, we should call the handlers that are
// interested to this event. In order for the handlers to be called, they must
// implement the same interface so we need to specify the function signature of
// an event handler. If a handler returns TRUE no other handler consumes the 
// event!
typedef b8 (*PFN_Event_Handler)(Event_Code code, void* sender, void* listener_inst, Event_Context data);

b8 event_startup();
void event_shutdown();

KOALA_API b8 event_register_listener(
    Event_Code code,
    void* listener,
    PFN_Event_Handler on_event);

KOALA_API b8 event_unregister_listener(
    Event_Code code,
    void* listener,
    PFN_Event_Handler on_event);

KOALA_API b8 event_fire(
    Event_Code code,
    void* sender,
    Event_Context context);
