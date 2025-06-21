#include "core/input.hpp"

#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

struct Keyboard_State {
    u8 keys[255];
};

struct Mouse_State {
    s16 x;
    s16 y;
    b8 buttons[(u16)Mouse_Button::MAX_BUTTONS];
};

struct Input_State {
    Keyboard_State keyboard_current;
    Keyboard_State keyboard_previous;
    Mouse_State mouse_current;
    Mouse_State mouse_previous;
};

internal Input_State* state_ptr = nullptr;

void input_startup(u64* mem_req, void* state) {

    *mem_req = sizeof(Input_State);

    if (state == nullptr) {
        return;
    }

    state_ptr = static_cast<Input_State*>(state);
    memory_zero(state_ptr, sizeof(Input_State));

    ENGINE_DEBUG("Input subsystem initialized");
}

void input_shutdown(void* state) {
    ENGINE_DEBUG("Input subsystem shutting down...");

	state_ptr = nullptr;
}

void input_update(f64 delta_time) {
    if (!state_ptr)
        return;

    memory_copy(
        &state_ptr->keyboard_previous,
        &state_ptr->keyboard_current,
        sizeof(Keyboard_State));

    memory_copy(
        &state_ptr->mouse_previous,
        &state_ptr->mouse_current,
        sizeof(Mouse_State));
}

b8 input_is_key_down(Keyboard_Key key) {
    if (!state_ptr)
        return FALSE;

    return state_ptr->keyboard_current.keys[(u16)key] == TRUE; // because in the state we save TRUE when key is down
}

b8 input_is_key_up(Keyboard_Key key) {
    if (!state_ptr)
        return TRUE;

    return state_ptr->keyboard_current.keys[(u16)key] == FALSE;
}

b8 input_was_key_down(Keyboard_Key key) {
    if (!state_ptr)
        return FALSE;

    return state_ptr->keyboard_previous.keys[(u16)key] == TRUE;
}

b8 input_was_key_up(Keyboard_Key key) {
    if (!state_ptr)
        return TRUE;

    return state_ptr->keyboard_previous.keys[(u16)key] == FALSE;
}

void input_process_key(
    Keyboard_Key key,
    u16 modifier_mask,
    b8 pressed) {
    if (state_ptr->keyboard_current.keys[(u16)key] != pressed) {
        state_ptr->keyboard_current.keys[(u16)key] = pressed;

        Event_Context context;
        context.data.u16[0] = static_cast<u16>(key);
        context.data.u16[1] = modifier_mask;

        event_fire(
            pressed
                ? Event_Code::KEY_PRESSED
                : Event_Code::KEY_RELEASED,
            nullptr,
            context);
    }
}

b8 input_is_button_down(Mouse_Button button) {
    if (!state_ptr)
        return FALSE;

    return state_ptr->mouse_current.buttons[(u16)button] == TRUE;
}

b8 input_is_button_up(Mouse_Button button) {
    if (!state_ptr)
        return TRUE;

    return state_ptr->mouse_current.buttons[(u16)button] == FALSE;
}

b8 input_was_button_down(Mouse_Button button) {
    if (!state_ptr)
        return FALSE;

    return state_ptr->mouse_previous.buttons[(u16)button] == TRUE;
}

b8 input_was_button_up(Mouse_Button button) {
    if (!state_ptr)
        return TRUE;

    return state_ptr->mouse_previous.buttons[(u16)button] == FALSE;
}

void input_get_current_mouse_position(s32* x, s32* y) {
    if (!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state_ptr->mouse_current.x;
    *y = state_ptr->mouse_current.y;
}

void input_get_previous_mouse_position(s32* x, s32* y) {
    if (!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state_ptr->mouse_previous.x;
    *y = state_ptr->mouse_previous.y;
}

void input_process_button(
    Mouse_Button button,
    b8 pressed) {
    if (state_ptr->mouse_current.buttons[(u16)button] != pressed) {
        state_ptr->mouse_current.buttons[(u16)button] = pressed;
        // ENGINE_DEBUG("Pressed button %d", button);

        Event_Context context;
        context.data.u16[0] = static_cast<u16>(button);

        event_fire(
            pressed
                ? Event_Code::BUTTON_PRESSED
                : Event_Code::BUTTON_RELEASED,
            nullptr,
            context);
    }
}

void input_process_mouse_move(s16 x, s16 y) {
    if (state_ptr->mouse_current.x != x || state_ptr->mouse_current.y != y) {
        // ENGINE_DEBUG("Mouse moved at (%d; %d)", x, y);

        state_ptr->mouse_current.x = x;
        state_ptr->mouse_current.y = y;

        Event_Context event;
        event.data.s16[0] = x;
        event.data.s16[1] = y;

        event_fire(
            Event_Code::MOUSE_MOVED,
            nullptr,
            event);
    }
}

void input_process_mouse_wheel_move(s8 z_delta) {
    // NOTE: No internal state to update

    Event_Context event;
    event.data.u8[0] = z_delta;
    // ENGINE_DEBUG("Scroll %s", z_delta == 1 ? "up" : "down");

    event_fire(
        Event_Code::MOUSE_WHEEL,
        nullptr,
        event);
}
