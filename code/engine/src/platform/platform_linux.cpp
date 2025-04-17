#include "platform.hpp"

#if ENGINE_PLATFORM_LINUX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_util.h>

#include "core/input.hpp"
#include "core/logger.hpp"

#include "containers/auto_array.hpp"

#define VK_USE_PLATFORM_XCB_KHR // Define this macro to avoid including vulkan_xcb.h
#include "renderer/vulkan/vulkan_types.hpp"
#include <vulkan/vulkan.h>

// NOTE: For the tutorial followed to write this platform layer see: https://www.youtube.com/watch?v=IPGROgWnI_8

#if _POSIC_X_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

struct Internal_State {
    xcb_connection_t* connection;
    int screen_number;
    xcb_screen_t* screen;
    xcb_window_t window_id; // Unique identifier
    xcb_void_cookie_t window_cookie;
    xcb_atom_t wm_delete_protocol;
    xcb_atom_t wm_protocols_property;
    xcb_key_symbols_t* key_symbols;
};

Keyboard_Key translate_key(xcb_keysym_t xcb_symbol);

b8 platform_startup(Platform_State* plat_state,
                    const char* application_name,
                    s32 x,
                    s32 y,
                    s32 width,
                    s32 height) {
    // Allocate and cold cast the internal state pointer to the application state
    plat_state->internal_state = malloc(sizeof(Internal_State));
    Internal_State* state = static_cast<Internal_State*>(plat_state->internal_state);

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
        .background_pixel = 0x00FFFF00, // alpha | red | green | blue
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

// Vulkan platform specific definitions
void platform_get_required_extensions(Auto_Array<const char*>* required_extensions) {
    ENGINE_INFO("Attaching XCB surface for LINUX platform");
    required_extensions->add("VK_KHR_xcb_surface");
}

b8 platform_create_vulkan_surface(Vulkan_Context* context, Platform_State* plat_state) {

    ENGINE_INFO("Creating Vulkan XCB surface...");
    Internal_State* state = static_cast<Internal_State*>(plat_state->internal_state);

    VkXcbSurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
    create_info.connection = state->connection;
    create_info.window = state->window_id;

    VK_ENSURE_SUCCESS(
        vkCreateXcbSurfaceKHR(
            context->instance,
            &create_info,
            context->allocator,
            &context->surface));

    ENGINE_INFO("Vulkan XCB surface created.");

    return TRUE;
}

b8 platform_message_pump(Platform_State* plat_state) {
    Internal_State* state = static_cast<Internal_State*>(plat_state->internal_state);

    xcb_generic_event_t* generic_event;

    b8 quit_flagged = FALSE;

    while ((generic_event = xcb_poll_for_event(state->connection))) {
        switch (XCB_EVENT_RESPONSE_TYPE(generic_event)) {
        case XCB_EXPOSE: {
        } break;
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE: {
            xcb_key_press_event_t* ev = reinterpret_cast<xcb_key_press_event_t*>(
                generic_event);

            xcb_keysym_t key_symbol = xcb_key_symbols_get_keysym(
                state->key_symbols,
                ev->detail,
                0);

            if (key_symbol == XCB_NO_SYMBOL) {
                ENGINE_WARN("No symbol for keycode %d", ev->detail);
                break;
            }

            // Extract modifiers
            u16 modifiers = ev->state;

            b8 shift_pressed = (modifiers & XCB_MOD_MASK_SHIFT);
            b8 ctrl_pressed = (modifiers & XCB_MOD_MASK_CONTROL) >> 2;

            // Not used modifiers
            // bool alt_pressed = modifiers & XCB_MOD_MASK_1;
            // bool super_pressed = modifiers & XCB_MOD_MASK_4;

            Keyboard_Key key = translate_key(key_symbol);

            // Treat control not as key but as a modifier only
            if (key == Keyboard_Key::LCONTROL ||
                key == Keyboard_Key::RCONTROL) {
                break;
            }

            input_process_key(
                key,
                modifiers,
                ev->response_type == XCB_KEY_PRESS);

        } break;
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE: {
            xcb_button_press_event_t* ev = reinterpret_cast<xcb_button_press_event_t*>(generic_event);
            b8 pressed = ev->response_type == XCB_BUTTON_PRESS;
            Mouse_Button button = Mouse_Button::MAX_BUTTONS;

            switch (ev->detail) {
            case XCB_BUTTON_INDEX_1:
                button = Mouse_Button::LEFT;
                break;
            case XCB_BUTTON_INDEX_2:
                button = Mouse_Button::RIGHT;
                break;
            case XCB_BUTTON_INDEX_3:
                button = Mouse_Button::MIDDLE;
                break;
            case XCB_BUTTON_INDEX_4:
            case XCB_BUTTON_INDEX_5: {
                if (pressed)
                    input_process_mouse_wheel_move(
                        ev->detail == XCB_BUTTON_INDEX_4
                            ? 1
                            : -1);
                break;
            }
            }

            if (button != Mouse_Button::MAX_BUTTONS) {
                input_process_button(
                    button,
                    pressed);
            }
        } break;
        case XCB_MOTION_NOTIFY: { // Mouse movement
            xcb_motion_notify_event_t* ev = reinterpret_cast<xcb_motion_notify_event_t*>(generic_event);

            input_process_mouse_move(
                ev->event_x,
                ev->event_y);
        } break;
        case XCB_CLIENT_MESSAGE: {
            xcb_client_message_event_t* cm = reinterpret_cast<xcb_client_message_event_t*>(
                generic_event);

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

void platform_shutdown(Platform_State* plat_state) {
    Internal_State* state = static_cast<Internal_State*>(plat_state->internal_state);

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

void platform_console_write(const char* message,
                            u8 colour) {
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

void platform_console_write_error(const char* message,
                                  u8 colour) {
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

// Clock is returned in seconds unit
f64 platform_get_absolute_time() {
    struct timespec now;
    // Clock monotonic is more robust compared to CLOCK_REALTIME
    // REALTIME will be impacted by changes in the system
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

void platform_sleep(u64 ms) { // in milliseconds
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
Keyboard_Key translate_key(xcb_keysym_t xcb_symbol) {
    switch (xcb_symbol) {
    case 0xff08: // Backspace
        return Keyboard_Key::BACKSPACE;

    case 0xff0d: // Enter
    case 0xff8d: // Enter numpad
        return Keyboard_Key::ENTER;
    case 0xff09: // Tab
        return Keyboard_Key::TAB;
    case 0xffe1:
        return Keyboard_Key::LSHIFT;
    case 0xffe2: // Shift
        return Keyboard_Key::RSHIFT;
    case 0xffe3:
        return Keyboard_Key::LCONTROL;
    case 0xffe4: // Control
        return Keyboard_Key::RCONTROL;

    case 0xffe9:
        return Keyboard_Key::LMETA;
    case 0xffea:
        return Keyboard_Key::RMETA;

    case 0xff13: // Pause
        return Keyboard_Key::PAUSE;
    case 0xffe5: // Caps Lock
        return Keyboard_Key::CAPS_LOCK;
    case 0xff1b: // Escape
        return Keyboard_Key::ESCAPE;
    case 0xff7e: // Mode Change
        return Keyboard_Key::MODECHANGE;

    case 0x0020: // Space
        return Keyboard_Key::SPACE;
    case 0xff55: // Page Up (Prior)
        return Keyboard_Key::PRIOR;
    case 0xff56: // Page Down (Next)
        return Keyboard_Key::NEXT;
    case 0xff57: // End
        return Keyboard_Key::END;
    case 0xff50: // Home
        return Keyboard_Key::HOME;
    case 0xff51: // Left Arrow
        return Keyboard_Key::LEFT;
    case 0xff52: // Up Arrow
        return Keyboard_Key::UP;
    case 0xff53: // Right Arrow
        return Keyboard_Key::RIGHT;
    case 0xff54: // Down Arrow
        return Keyboard_Key::DOWN;
    case 0xff60: // Select
        return Keyboard_Key::SELECT;
    case 0xff61: // Print
        return Keyboard_Key::PRINT;
    case 0xff62: // Execute
        return Keyboard_Key::EXECUTE;

    case 0xff63: // Insert
        return Keyboard_Key::INSERT;
    case 0xffff: // Delete
        return Keyboard_Key::DELETE;
    case 0xff6a: // Help
        return Keyboard_Key::HELP;

    case 0xffeb:
        return Keyboard_Key::LWIN;
    case 0xffec:
        return Keyboard_Key::RWIN;

    case 0x0030: // 0
    case 0xffb0: // 0
        return Keyboard_Key::NUMPAD0;
    case 0x0031: // 1
        return Keyboard_Key::NUMPAD1;
    case 0x0032: // 2
        return Keyboard_Key::NUMPAD2;
    case 0x0033: // 3
        return Keyboard_Key::NUMPAD3;
    case 0x0034: // 4
        return Keyboard_Key::NUMPAD4;
    case 0x0035: // 5
        return Keyboard_Key::NUMPAD5;
    case 0x0036: // 6
        return Keyboard_Key::NUMPAD6;
    case 0x0037: // 7
        return Keyboard_Key::NUMPAD7;
    case 0x0038: // 8
        return Keyboard_Key::NUMPAD8;
    case 0x0039: // 9
        return Keyboard_Key::NUMPAD9;

    case 0x00d7: // Multiply
    case 0xffaa: // Multiply numpad
        return Keyboard_Key::MULTIPLY;
    case 0xffab: // Add
        return Keyboard_Key::ADD;
    case 0xffac:
        return Keyboard_Key::SEPARATOR;
    case 0xffad: // Subtract
        return Keyboard_Key::SUBTRACT;
    case 0xffae: // Decimal
        return Keyboard_Key::DECIMAL;
    case 0xffaf: // Divide
        return Keyboard_Key::DIVIDE;

    case 0xffbe: // F1
        return Keyboard_Key::F1;
    case 0xffbf: // F2
        return Keyboard_Key::F2;
    case 0xffc0: // F3
        return Keyboard_Key::F3;
    case 0xffc1: // F4
        return Keyboard_Key::F4;
    case 0xffc2: // F5
        return Keyboard_Key::F5;
    case 0xffc3: // F6
        return Keyboard_Key::F6;
    case 0xffc4: // F7
        return Keyboard_Key::F7;
    case 0xffc5: // F8
        return Keyboard_Key::F8;
    case 0xffc6: // F9
        return Keyboard_Key::F9;
    case 0xffc7: // F10
        return Keyboard_Key::F10;
    case 0xffc8: // F11
        return Keyboard_Key::F11;
    case 0xffc9: // F12
        return Keyboard_Key::F12;
    case 0xffca: // F13
        return Keyboard_Key::F13;
    case 0xffcb: // F14
        return Keyboard_Key::F14;
    case 0xffcc: // F15
        return Keyboard_Key::F15;
    case 0xffcd: // F16
        return Keyboard_Key::F16;
    case 0xffce: // F17
        return Keyboard_Key::F17;
    case 0xffcf: // F18
        return Keyboard_Key::F18;
    case 0xffd0: // F19
        return Keyboard_Key::F19;
    case 0xffd1: // F20
        return Keyboard_Key::F20;
    case 0xffd2: // F21
        return Keyboard_Key::F21;
    case 0xffd3: // F22
        return Keyboard_Key::F22;
    case 0xffd4: // F23
        return Keyboard_Key::F23;
    case 0xffd5: // F24
        return Keyboard_Key::F24;

    case 0xff7f:
        return Keyboard_Key::NUMLOCK;
    case 0xff14:
        return Keyboard_Key::SCROLL;
    case 0x003d:
        return Keyboard_Key::NUMPAD_EQUAL;
    case 0xff67:
        return Keyboard_Key::RMENU;

    case 0x003b:
        return Keyboard_Key::SEMICOLON;
    case 0x002b:
        return Keyboard_Key::PLUS;
    case 0x002c:
        return Keyboard_Key::COMMA;
    case 0x002d:
        return Keyboard_Key::MINUS;
    case 0x002e:
        return Keyboard_Key::PERIOD;
    case 0x002f:
        return Keyboard_Key::SLASH;
    case 0x0060:
        return Keyboard_Key::TILDE;

    case 0x0041: // A
    case 0x0061: // a
        return Keyboard_Key::A;
    case 0x0042: // B
    case 0x0062: // b
        return Keyboard_Key::B;
    case 0x0043: // C
    case 0x0063: // c
        return Keyboard_Key::C;
    case 0x0044: // D
    case 0x0064: // d
        return Keyboard_Key::D;
    case 0x0045: // E
    case 0x0065: // e
        return Keyboard_Key::E;
    case 0x0046: // F
    case 0x0066: // f
        return Keyboard_Key::F;
    case 0x0047: // G
    case 0x0067: // G
        return Keyboard_Key::G;
    case 0x0048: // H
    case 0x0068: // H
        return Keyboard_Key::H;
    case 0x0049: // I
    case 0x0069: // i
        return Keyboard_Key::I;
    case 0x004a: // J
    case 0x006a: // j
        return Keyboard_Key::J;
    case 0x004b: // K
    case 0x006b: // k
        return Keyboard_Key::K;
    case 0x004c: // L
    case 0x006c: // l
        return Keyboard_Key::L;
    case 0x004d: // M
    case 0x006d: // m
        return Keyboard_Key::M;
    case 0x004e: // N
    case 0x006e: // n
        return Keyboard_Key::N;
    case 0x004f: // O
    case 0x006f: // o
        return Keyboard_Key::O;
    case 0x0050: // P
    case 0x0070: // p
        return Keyboard_Key::P;
    case 0x0051: // Q
    case 0x0071: // q
        return Keyboard_Key::Q;
    case 0x0052: // R
    case 0x0072: // r
        return Keyboard_Key::R;
    case 0x0053: // S
    case 0x0073: // s
        return Keyboard_Key::S;
    case 0x0054: // T
    case 0x0074: // T
        return Keyboard_Key::T;
    case 0x0055: // U
    case 0x0075: // U
        return Keyboard_Key::U;
    case 0x0056: // V
    case 0x0076: // V
        return Keyboard_Key::V;
    case 0x0057: // W
    case 0x0077: // W
        return Keyboard_Key::W;
    case 0x0058: // X
    case 0x0078: // X
        return Keyboard_Key::X;
    case 0x0059: // Y
    case 0x0079: // Y
        return Keyboard_Key::Y;
    case 0x005a: // Z
    case 0x007a: // Z
        return Keyboard_Key::Z;

    default:
        return Keyboard_Key::UNKNOWN;
    };
}

#endif
