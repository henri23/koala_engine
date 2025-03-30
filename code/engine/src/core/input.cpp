#include "core/input.hpp"

#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

struct keyboard_state {
    u8 keys[255];
};

struct mouse_state {
    s16 x;
    s16 y;
    b8 buttons[(u16)mouse_button::MAX_BUTTONS];
};

struct input_state {
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;
};

internal b8 is_initialized = FALSE;
internal input_state state;

void input_startup() {
    memory_zero(&state, sizeof(state));

    is_initialized = TRUE;

    ENGINE_DEBUG("Input subsystem initialized");
}

void input_shutdown() {
    ENGINE_DEBUG("Input subsystem shutting down...");

    is_initialized = FALSE;
}

void input_update(f64 delta_time) {
    if (!is_initialized)
        return;

    memory_copy(
        &state.keyboard_previous,
        &state.keyboard_current,
        sizeof(keyboard_state));

    memory_copy(
        &state.mouse_previous,
        &state.mouse_current,
        sizeof(mouse_state));
}

b8 input_is_key_down(keyboard_key key) {
    if (!is_initialized) return FALSE;

    return state.keyboard_current.keys[(u16)key] == TRUE;  // because in the state we save TRUE when key is down
}

b8 input_is_key_up(keyboard_key key) {
    if (!is_initialized) return TRUE;

    return state.keyboard_current.keys[(u16)key] == FALSE;
}

b8 input_was_key_down(keyboard_key key) {
    if (!is_initialized) return FALSE;

    return state.keyboard_previous.keys[(u16)key] == TRUE;
}

b8 input_was_key_up(keyboard_key key) {
    if (!is_initialized) return TRUE;

    return state.keyboard_previous.keys[(u16)key] == FALSE;
}

void input_process_key(keyboard_key key, b8 pressed) {
    if (state.keyboard_current.keys[(u16)key] != pressed) {
        state.keyboard_current.keys[(u16)key] = pressed;

        event_context context;
        context.data.u16[0] = static_cast<u16>(key);

        event_fire(
            pressed
                ? event_code::KEY_PRESSED
                : event_code::KEY_RELEASED,
            nullptr,
            context);
    }
}

b8 input_is_button_down(mouse_button button) {
    if (!is_initialized) return FALSE;

    return state.mouse_current.buttons[(u16)button] == TRUE;
}

b8 input_is_button_up(mouse_button button) {
    if (!is_initialized) return TRUE;

    return state.mouse_current.buttons[(u16)button] == FALSE;
}

b8 input_was_button_down(mouse_button button) {
    if (!is_initialized) return FALSE;

    return state.mouse_previous.buttons[(u16)button] == TRUE;
}

b8 input_was_button_up(mouse_button button) {
    if (!is_initialized) return TRUE;

    return state.mouse_previous.buttons[(u16)button] == FALSE;
}

void input_get_current_mouse_position(s32* x, s32* y) {
    if (!is_initialized) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input_get_previous_mouse_position(s32* x, s32* y) {
    if (!is_initialized) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}

void input_process_button(mouse_button button, b8 pressed) {
    if (state.mouse_current.buttons[(u16)button] != pressed) {
        state.mouse_current.buttons[(u16)button] = pressed;

        ENGINE_DEBUG("Pressed button %d", button);

        event_context context;
        context.data.u16[0] = static_cast<u16>(button);

        event_fire(
            pressed
                ? event_code::BUTTON_PRESSED
                : event_code::BUTTON_RELEASED,
            nullptr,
            context);
    }
}

void input_process_mouse_move(s16 x, s16 y) {
    if (state.mouse_current.x != x || state.mouse_current.y != y) {
        // ENGINE_DEBUG("Mouse moved at (%d; %d)", x, y);

        state.mouse_current.x = x;
        state.mouse_current.y = y;

        event_context event;
        event.data.s16[0] = x;
        event.data.s16[1] = y;

        event_fire(
            event_code::MOUSE_MOVED,
            nullptr,
            event);
    }
}

void input_process_mouse_wheel_move(s8 z_delta) {
    // NOTE: No internal state to update

    event_context event;
    event.data.u8[0] = z_delta;
    event_fire(
        event_code::MOUSE_WHEEL,
        nullptr,
        event);
}
