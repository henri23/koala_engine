#include "platform.hpp"

#if ENGINE_PLATFORM_LINUX

// NOTE: For the tutorial followed to write this platform layer see: https://www.youtube.com/watch?v=IPGROgWnI_8
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_util.h>

#include "core/input.hpp"
#include "core/logger.hpp"

#if _POSIC_X_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

struct internal_state {
    xcb_connection_t* connection;
    int screen_number;
    xcb_screen_t* screen;
    xcb_window_t window_id;  // Unique identifier
    xcb_void_cookie_t window_cookie;
    xcb_atom_t wm_delete_protocol;
    xcb_atom_t wm_protocols_property;
    xcb_key_symbols_t* key_symbols;
};

keyboard_key translate_key(xcb_keysym_t xcb_symbol);

b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    s32 x,
    s32 y,
    s32 width,
    s32 height) {
    // Allocate and cold cast the internal state pointer to the application state
    plat_state->internal_state = malloc(sizeof(internal_state));
    internal_state* state = static_cast<internal_state*>(plat_state->internal_state);

    // Establish xcb connection with the preferred (current) screen
    state->connection = xcb_connect(nullptr, &state->screen_number);

    if (xcb_connection_has_error(state->connection)) {
        ENGINE_FATAL("Error while establishing XCB connection");
        return FALSE;
    }

    xcb_intern_atom_cookie_t intern_atom_cookie;
    xcb_intern_atom_reply_t* intern_atom_reply;

    intern_atom_cookie = xcb_intern_atom(state->connection, 0, 12, "WM_PROTOCOLS");
    intern_atom_reply = xcb_intern_atom_reply(state->connection, intern_atom_cookie, nullptr);
    state->wm_protocols_property = intern_atom_reply->atom;
    free(intern_atom_reply);

    intern_atom_cookie = xcb_intern_atom(state->connection, 0, 16, "WM_DELETE_WINDOW");
    intern_atom_reply = xcb_intern_atom_reply(state->connection, intern_atom_cookie, nullptr);
    state->wm_delete_protocol = intern_atom_reply->atom;
    free(intern_atom_reply);

    // Get screen from screen number using the auxilliary function.
    // Alternativelly we could loop over all the screens with an iterator and get the screen with the proper id
    state->screen = xcb_aux_get_screen(state->connection, state->screen_number);

    // NOTE: By defaul the X server will not send any event so we need to specify which events we want to receive
    u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                       XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                       XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                       XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    // By using the object it is easier not to worry about the proper ordering of properties
    xcb_create_window_value_list_t value_list = {
        .background_pixel = 0x00FFFF00,  // alpha | red | green | blue
        .event_mask = event_values};

    // Generate unique identifier for this window
    state->window_id = xcb_generate_id(state->connection);

    state->window_cookie = xcb_create_window_aux(
        state->connection,
        state->screen->root_depth,
        state->window_id,
        state->screen->root,
        x,
        y,
        width,
        height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        state->screen->root_visual,
        XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        &value_list);

    // Change the title
    xcb_icccm_set_wm_name(
        state->connection,
        state->window_id,
        XCB_ATOM_STRING,
        8,
        strlen(application_name),
        application_name);

    xcb_map_window(state->connection, state->window_id);

    s32 stream_flush = xcb_flush(state->connection);
    if (stream_flush <= 0) {
        ENGINE_FATAL("Error while flushing the stream: %d", stream_flush);
        return FALSE;
    }

    state->key_symbols = xcb_key_symbols_alloc(state->connection);
    if (!state->key_symbols) {
        ENGINE_FATAL("Error while creating key map");
        return FALSE;
    }

    ENGINE_DEBUG("Platform layer with LINUX interface initialized")

    return TRUE;
}

b8 platform_message_pump(platform_state* plat_state) {
    internal_state* state = static_cast<internal_state*>(plat_state->internal_state);

    xcb_generic_event_t* generic_event;

    b8 quit_flagged = FALSE;

    while ((generic_event = xcb_poll_for_event(state->connection))) {
        switch (XCB_EVENT_RESPONSE_TYPE(generic_event)) {
            case XCB_EXPOSE: {
                ENGINE_DEBUG("Expose");
            } break;
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {
                xcb_key_press_event_t* ev = reinterpret_cast<xcb_key_press_event_t*>(generic_event);

                xcb_keysym_t key_symbol = xcb_key_symbols_get_keysym(
                    state->key_symbols,
                    ev->detail,
                    0);

                if (key_symbol == XCB_NO_SYMBOL)
                    ENGINE_WARN("No symbol for keycode %d", ev->detail);

                keyboard_key key_name = translate_key(key_symbol);

                input_process_key(key_name, ev->response_type == XCB_KEY_PRESS);

                // Map event details (code) into the internal symbol of the engine
                // Publish event for key pressed/released
                // TODO: Create internal enum for keys
                // TODO Create mapper from linux codes to internal symbols

            } break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {
            } break;
            case XCB_MOTION_NOTIFY: {  // Mouse movement
            } break;
            case XCB_CLIENT_MESSAGE: {
                xcb_client_message_event_t* cm = (xcb_client_message_event_t*)generic_event;

                if (cm->data.data32[0] == state->wm_delete_protocol) {
                    quit_flagged = TRUE;
                }
            } break;
            default:
                break;
        }

        // NOTE:  We need to free the event after every event because the events will be dynamically allocated
        //        by XCB when we poll for new events. They are dynamically allocated because the events can
        //        differ in size so they cannot be stack allocated. We can fix this for internal events with unions
        free(generic_event);
    }

    return !quit_flagged;
}

void platform_shutdown(platform_state* plat_state) {
    internal_state* state = static_cast<internal_state*>(plat_state->internal_state);

    xcb_key_symbols_free(state->key_symbols);
    xcb_destroy_window(state->connection, state->window_id);
    xcb_disconnect(state->connection);

    platform_free(plat_state->internal_state, TRUE);

    ENGINE_DEBUG("Platform layer shutting down...");
}

void* platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platform_free(void* block, b8 aligned) {
    free(block);
}

void* platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform_move_memory(void* dest, const void* source, u64 size) {
    return memmove(dest, source, size);
}

void* platform_set_memory(void* dest, s32 value, u64 size) {
    return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 colour) {
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

void platform_console_write_error(const char* message, u8 colour) {
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

f64 platform_get_absolute_time() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

void platform_sleep(u64 ms) {
#if _POSIC_X_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
#else
    if (ms >= 1000) {
        sleep(ms / 1000);
    }
    usleep((ms % 1000) * 1000);
#endif
}

// Definitons taken from <X11/keysymdef.h> from LATIN1 section
keyboard_key translate_key(xcb_keysym_t xcb_symbol) {
    switch (xcb_symbol) {
        case 0xff08:  // Backspace
            return keyboard_key::BACKSPACE;
        case 0xff0d:  // Enter
            return keyboard_key::ENTER;
        case 0xff09:  // Tab
            return keyboard_key::TAB;
        case 0xffe1:
            return keyboard_key::LSHIFT;
        case 0xffe2:  // Shift
            return keyboard_key::RSHIFT;
        case 0xffe3:
            return keyboard_key::LCONTROL;
        case 0xffe4:  // Control
            return keyboard_key::RCONTROL;

        case 0xff13:  // Pause
            return keyboard_key::PAUSE;
        case 0xffe5:  // Caps Lock
            return keyboard_key::CAPS_LOCK;
        case 0xff1b:  // Escape
            return keyboard_key::ESCAPE;
        case 0xff7e:  // Mode Change
            return keyboard_key::MODECHANGE;

        case 0x0020:  // Space
            return keyboard_key::SPACE;
        case 0xff55:  // Page Up (Prior)
            return keyboard_key::PRIOR;
        case 0xff56:  // Page Down (Next)
            return keyboard_key::NEXT;
        case 0xff57:  // End
            return keyboard_key::END;
        case 0xff50:  // Home
            return keyboard_key::HOME;
        case 0xff51:  // Left Arrow
            return keyboard_key::LEFT;
        case 0xff52:  // Up Arrow
            return keyboard_key::UP;
        case 0xff53:  // Right Arrow
            return keyboard_key::RIGHT;
        case 0xff54:  // Down Arrow
            return keyboard_key::DOWN;
        case 0xff60:  // Select
            return keyboard_key::SELECT;
        case 0xff61:  // Print
            return keyboard_key::PRINT;
        case 0xff62:  // Execute
            return keyboard_key::EXECUTE;


        case 0xff63:  // Insert
            return keyboard_key::INSERT;
        case 0xffff:  // Delete
            return keyboard_key::DELETE;
        case 0xff6a:  // Help
            return keyboard_key::HELP;


        case 0xffeb :
            return keyboard_key::LWIN;
        case 0xffec:
            return keyboard_key::RWIN;


        case 0xffb0:  // 0
            return keyboard_key::NUMPAD0;
        case 0xffb1:  // 1
            return keyboard_key::NUMPAD1;
        case 0xffb2:  // 2
            return keyboard_key::NUMPAD2;
        case 0xffb3:  // 3
            return keyboard_key::NUMPAD3;
        case 0xffb4:  // 4
            return keyboard_key::NUMPAD4;
        case 0xffb5:  // 5
            return keyboard_key::NUMPAD5;
        case 0xffb6:  // 6
            return keyboard_key::NUMPAD6;
        case 0xffb7:  // 7
            return keyboard_key::NUMPAD7;
        case 0xffb8:  // 8
            return keyboard_key::NUMPAD8;
        case 0xffb9:  // 9
            return keyboard_key::NUMPAD9;

        case 0x00d7:  // Multiply
            return keyboard_key::MULTIPLY;
        case 0xffab:  // Add
            return keyboard_key::ADD;
        case 0xffac:
            return keyboard_key::SEPARATOR;
        case 0xffad:  // Subtract
            return keyboard_key::SUBTRACT;
        case 0xffae:  // Decimal
            return keyboard_key::DECIMAL;
        case 0xffaf:  // Divide
            return keyboard_key::DIVIDE;


        case 0xffbe:  // F1
            return keyboard_key::F1;
        case 0xffbf:  // F2
            return keyboard_key::F2;
        case 0xffc0:  // F3
            return keyboard_key::F3;
        case 0xffc1:  // F4
            return keyboard_key::F4;
        case 0xffc2:  // F5
            return keyboard_key::F5;
        case 0xffc3:  // F6
            return keyboard_key::F6;
        case 0xffc4:  // F7
            return keyboard_key::F7;
        case 0xffc5:  // F8
            return keyboard_key::F8;
        case 0xffc6:  // F9
            return keyboard_key::F9;
        case 0xffc7:  // F10
            return keyboard_key::F10;
        case 0xffc8:  // F11
            return keyboard_key::F11;
        case 0xffc9:  // F12
            return keyboard_key::F12;
        case 0xffca:  // F13
            return keyboard_key::F13;
        case 0xffcb:  // F14
            return keyboard_key::F14;
        case 0xffcc:  // F15
            return keyboard_key::F15;
        case 0xffcd:  // F16
            return keyboard_key::F16;
        case 0xffce:  // F17
            return keyboard_key::F17;
        case 0xffcf:  // F18
            return keyboard_key::F18;
        case 0xffd0:  // F19
            return keyboard_key::F19;
        case 0xffd1:  // F20
            return keyboard_key::F20;
        case 0xffd2:  // F21
            return keyboard_key::F21;
        case 0xffd3:  // F22
            return keyboard_key::F22;
        case 0xffd4:  // F23
            return keyboard_key::F23;
        case 0xffd5:  // F24
            return keyboard_key::F24;


        case 0xff7f:
            return keyboard_key::NUMLOCK;
        case 0xff14:
            return keyboard_key::SCROLL;
        case 0xffbd:
            return keyboard_key::NUMPAD_EQUAL;
        case 0xff67:
            return keyboard_key::RMENU;


        case 0x003b:
            return keyboard_key::SEMICOLON;
        case 0x002b:
            return keyboard_key::PLUS;
        case 0x002c:
            return keyboard_key::COMMA;
        case 0x002d:
            return keyboard_key::MINUS;
        case 0x002e:
            return keyboard_key::PERIOD;
        case 0x002f:
            return keyboard_key::SLASH;
        case 0x0060:
            return keyboard_key::TILDE;


        case 0x0041:  // A
        case 0x0061:  // a
            return keyboard_key::A;
        case 0x0042:  // B
        case 0x0062:  // b
            return keyboard_key::B;
        case 0x0043:  // C
        case 0x0063:  // c
            return keyboard_key::C;
        case 0x0044:  // D
        case 0x0064:  // d
            return keyboard_key::D;
        case 0x0045:  // E
        case 0x0065:  // e
            return keyboard_key::E;
        case 0x0046:  // F
        case 0x0066:  // f
            return keyboard_key::F;
        case 0x0047:  // G
        case 0x0067:  // G
            return keyboard_key::G;
        case 0x0048:  // H
        case 0x0068:  // H
            return keyboard_key::H;
        case 0x0049:  // I
        case 0x0069:  // i
            return keyboard_key::I;
        case 0x004a:  // J
        case 0x006a:  // j
            return keyboard_key::J;
        case 0x004b:  // K
        case 0x006b:  // k
            return keyboard_key::K;
        case 0x004c:  // L
        case 0x006c:  // l
            return keyboard_key::L;
        case 0x004d:  // M
        case 0x006d:  // m
            return keyboard_key::M;
        case 0x004e:  // N
        case 0x006e:  // n
            return keyboard_key::N;
        case 0x004f:  // O
        case 0x006f:  // o
            return keyboard_key::O;
        case 0x0050:  // P
        case 0x0070:  // p
            return keyboard_key::P;
        case 0x0051:  // Q
        case 0x0071:  // q
            return keyboard_key::Q;
        case 0x0052:  // R
        case 0x0072:  // r
            return keyboard_key::R;
        case 0x0053:  // S
        case 0x0073:  // s
            return keyboard_key::S;
        case 0x0054:  // T
        case 0x0074:  // T
            return keyboard_key::T;
        case 0x0055:  // U
        case 0x0075:  // U
            return keyboard_key::U;
        case 0x0056:  // V
        case 0x0076:  // V
            return keyboard_key::V;
        case 0x0057:  // W
        case 0x0077:  // W
            return keyboard_key::W;
        case 0x0058:  // X
        case 0x0078:  // X
            return keyboard_key::X;
        case 0x0059:  // Y
        case 0x0079:  // Y
            return keyboard_key::Y;
        case 0x005a:  // Z
        case 0x007a:  // Z
            return keyboard_key::Z;

        default:
            return keyboard_key::UNKNOWN;
    };
}

#endif
