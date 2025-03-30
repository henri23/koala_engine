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
    b8 buttons[(u32)mouse_button::MAX_BUTTONS];
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
        &state.mouse_previous,
        sizeof(mouse_state));
}

b8 input_is_key_down(keyboard_key key);
b8 input_is_key_up(keyboard_key key);
b8 input_was_key_down(keyboard_key key);
b8 input_was_key_up(keyboard_key key);

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

b8 input_is_button_down(mouse_button button);
b8 input_is_button_up(mouse_button button);
b8 input_was_button_down(mouse_button button);
b8 input_was_button_up(mouse_button button);
void input_get_current_mouse_position(s32 x, s32 y);
void input_get_previous_mouse_position(s32 x, s32 y);

void input_process_button(mouse_button button, b8 pressed) {
    if (state.mouse_current.buttons[(u16)button] != pressed) {
        state.mouse_current.buttons[(u16)button] = pressed;

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

void input_process_mouse_move(s16 x, s16 y);
void input_process_mouse_move(s8 z_delta);
