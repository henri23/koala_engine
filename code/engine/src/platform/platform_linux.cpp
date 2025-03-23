#include "platform.hpp"

#if ENGINE_PLATFORM_LINUX

// NOTE: For the tutorial followed to write this platform layer see: https://www.youtube.com/watch?v=IPGROgWnI_8
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_util.h>

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
    internal_state* state = static_cast<internal_state*>(plat_state->internal_state);

    // Establish xcb connection with the preferred (current) screen
    state->connection = xcb_connect(NULL, &state->screen_number);

    if (xcb_connection_has_error(state->connection)) {
        ENGINE_FATAL("Error while establishing XCB connection");
        return FALSE;
    }

    xcb_intern_atom_cookie_t intern_atom_cookie;
    xcb_intern_atom_reply_t* intern_atom_reply;

    intern_atom_cookie = xcb_intern_atom(state->connection, 0, 12, "WM_PROTOCOLS");
    intern_atom_reply = xcb_intern_atom_reply(state->connection, intern_atom_cookie, NULL);
    state->wm_protocols_property = intern_atom_reply->atom;
    free(intern_atom_reply);

    intern_atom_cookie = xcb_intern_atom(state->connection, 0, 16, "WM_DELETE_WINDOW");
    intern_atom_reply = xcb_intern_atom_reply(state->connection, intern_atom_cookie, NULL);
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

    ENGINE_DEBUG("Platform layer with LINUX interface initialized")

    return TRUE;
}

b8 platform_read_events(platform_state* plat_state) {
    internal_state* state = static_cast<internal_state*>(plat_state->internal_state);

    xcb_generic_event_t* generic_event;
    xcb_client_message_event_t* cm;

    b8 quit_flagged = FALSE;

    while ((generic_event = xcb_poll_for_event(state->connection))) {

        switch (XCB_EVENT_RESPONSE_TYPE(generic_event)) {
            case XCB_EXPOSE: {
                ENGINE_DEBUG("Expose");
            } break;
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {
            } break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {
            } break;
            case XCB_MOTION_NOTIFY: {  // Mouse movement
            } break;
            case XCB_CLIENT_MESSAGE: {
                cm = (xcb_client_message_event_t*)generic_event;

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

#endif
