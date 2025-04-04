#pragma once

#include "defines.hpp"

enum class mouse_button : u16 {
    LEFT,
    RIGHT,
    MIDDLE,

    MAX_BUTTONS
};

// NOTE:  Linux uses 16 bits for key symbols. Defined in keysymdef.h inside X11library
//        The internal codes will be defined the same as the windows keys so at least
//        we get one mapping less because in windows they will be the same
enum class keyboard_key : u16 {
    UNKNOWN = 0x00,

    BACKSPACE = 0x08,
    ENTER = 0x0D,
    TAB = 0x09,
    SHIFT = 0x10,
    CONTROL = 0x11,

    PAUSE = 0x13,
    CAPS_LOCK = 0x14,

    ESCAPE = 0x1B,

    CONVERT = 0x1C,
    NONCONVERT = 0x1D,
    ACCEPT = 0x1E,
    MODECHANGE = 0x1F,

    SPACE = 0x20,
    PRIOR = 0x21,  // Pageup
    NEXT = 0x22,   // Pagedown
    END = 0x23,
    HOME = 0x24,
    LEFT = 0x25,
    UP = 0x26,
    RIGHT = 0x27,
    DOWN = 0x28,
    SELECT = 0x29,
    PRINT = 0x2A,
    EXECUTE = 0x2B,
    SNAPSHOT = 0x2C,
    INSERT = 0x2D,
    DELETE = 0x2E,
    HELP = 0x2F,

    // Letters refer to lowercase, no need to map uppercase letters
    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,

    LWIN = 0x5B,
    RWIN = 0x5C,
    APPS = 0x5D,

    SLEEP = 0x5F,

    NUMPAD0 = 0x60,
    NUMPAD1 = 0x61,
    NUMPAD2 = 0x62,
    NUMPAD3 = 0x63,
    NUMPAD4 = 0x64,
    NUMPAD5 = 0x65,
    NUMPAD6 = 0x66,
    NUMPAD7 = 0x67,
    NUMPAD8 = 0x68,
    NUMPAD9 = 0x69,

    MULTIPLY = 0x6A,
    ADD = 0x6B,
    SEPARATOR = 0x6C,
    SUBTRACT = 0x6D,
    DECIMAL = 0x6E,
    DIVIDE = 0x6F,

    F1 = 0x70,
    F2 = 0x71,
    F3 = 0x72,
    F4 = 0x73,
    F5 = 0x74,
    F6 = 0x75,
    F7 = 0x76,
    F8 = 0x77,
    F9 = 0x78,
    F10 = 0x79,
    F11 = 0x7A,
    F12 = 0x7B,
    F13 = 0x7C,
    F14 = 0x7D,
    F15 = 0x7E,
    F16 = 0x7F,
    F17 = 0x80,
    F18 = 0x81,
    F19 = 0x82,
    F20 = 0x83,
    F21 = 0x84,
    F22 = 0x85,
    F23 = 0x86,
    F24 = 0x87,

    NUMLOCK = 0x90,
    SCROLL = 0x91,

    NUMPAD_EQUAL = 0x92,

    LSHIFT = 0xA0,
    RSHIFT = 0xA1,
    LCONTROL = 0xA2,
    RCONTROL = 0xA3,
    LMENU = 0xA4,
    RMENU = 0xA5,

    SEMICOLON = 0xBA,
    PLUS = 0xBB,
    COMMA = 0xBC,
    MINUS = 0xBD,
    PERIOD = 0xBE,
    SLASH = 0xBF,
    TILDE = 0xC0,

    LMETA = 0xC1,
    RMETA = 0xC2,

    MAX_KEYS
};

// Applying a bitwise and with these enum entries will yield whether a 
// modifier was being pressed when the key event was published
enum key_modifiers {
    KEY_MODIFIER_MASK_SHIFT = 1,
    KEY_MODIFIER_MASK_LOCK = 2,
    KEY_MODIFIER_MASK_CONTROL = 4,
    KEY_MODIFIER_MASK_ALT = 8,
    KEY_MODIFIER_MASK_SUPER = 64
};

// Functions called by the application layer to initialize the input subsystem
void input_startup();
void input_shutdown();
void input_update(f64 delta_time);

// NOTE: Functions to set the state (Called by platform layer)
void input_process_key(
    keyboard_key key,
    u16 modifier_mask,
    b8 pressed);

void input_process_button(
    mouse_button button,
    b8 pressed);

void input_process_mouse_move(
    s16 x,
    s16 y);

void input_process_mouse_wheel_move(s8 z_delta);

// NOTE: Functions to query the input state
KOALA_API b8 input_is_key_down(keyboard_key key);
KOALA_API b8 input_is_key_up(keyboard_key key);
KOALA_API b8 input_was_key_down(keyboard_key key);
KOALA_API b8 input_was_key_up(keyboard_key key);

KOALA_API b8 input_is_button_down(mouse_button button);
KOALA_API b8 input_is_button_up(mouse_button button);
KOALA_API b8 input_was_button_down(mouse_button button);
KOALA_API b8 input_was_button_up(mouse_button button);

// Need to pass pointer because we need to return 2 values
KOALA_API void input_get_current_mouse_position(s32* x, s32* y);
KOALA_API void input_get_previous_mouse_position(s32* x, s32* y);
