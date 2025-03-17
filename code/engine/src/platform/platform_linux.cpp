#include <xcb/xproto.h>

#include "platform.hpp"

#if ENGINE_PLATFORM_LINUX

// NOTE: For the tutorial followed to write this platform layer see: https://www.youtube.com/watch?v=IPGROgWnI_8
#include <xcb/xcb.h>
#include <xcb/xcb_util.h>

#include "core/logger.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct internal_state {
};

b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    s32 x,
    s32 y,
    s32 width,
    s32 height) {
    // Allocate and cold cast the internal state pointer to the application state
    plat_state->internal_state = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)plat_state->internal_state;

    // Establish xcb connection with the preferred (current) screen
    int screen_number;
    xcb_connection_t *connection = xcb_connect(NULL, &screen_number);

    if (xcb_connection_has_error(connection)) {
        ENGINE_FATAL("Error while establishing XCB connection");
        return FALSE;
    }

    // Get screen from screen number using the auxilliary function.
    // Alternativelly we could loop over all the screens with an iterator and get the screen with the proper id
    xcb_screen_t* screen = xcb_aux_get_screen(connection, screen_number);

    // Generate unique identifier for this window
    xcb_window_t window = xcb_generate_id(connection);

    // If the event values is not declared the window will not show
    u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                       XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                       XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                       XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    u32 value_list[] = {screen->black_pixel, event_values};

    xcb_void_cookie_t cookie = xcb_create_window(
        connection,
        screen->root_depth,
        window,
        screen->root,
        x,
        y,
        width,
        height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        value_list);

    // Change the title
    xcb_change_property(
        connection,
        XCB_PROP_MODE_REPLACE,
        window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,  // data should be viewed 8 bits at a time
        strlen(application_name),
        application_name);

    xcb_map_window(connection, window);

    xcb_flush(connection);

    return TRUE;
}

#endif
