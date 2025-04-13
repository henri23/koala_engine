#include "application.hpp"

#include "core/absolute_clock.hpp"
#include "core/event.hpp"
#include "core/input.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "game_types.hpp"
#include "platform/platform.hpp"

#include "renderer/renderer_frontend.hpp"
#include "renderer/renderer_types.inl"

#define TARGET_FRAME_TIME (f64)1.0f / 60

struct Application_State {
    Game* game_inst;
    b8 is_running;
    b8 is_suspended;
    Platform_State platform_state;
    s16 width;
    s16 height;
    Absolute_Clock clock;
};

internal b8 is_initialized = FALSE;
internal Application_State application_state;

// Forward declared event handlers. Implemented below
b8 application_on_event(
    Event_Code code,
    void* sender,
    void* listener,
    Event_Context data);

b8 application_on_key(
    Event_Code code,
    void* sender,
    void* listener,
    Event_Context data);

b8 application_initialize(Game* game_inst) {
    // TODO: Automate process of susbsytem initialization in a queue with dependency order optimization

    // Protect the application state from being initialized multiple times
    if (is_initialized) {
        ENGINE_ERROR("Engine subsystems are already initialized");
        return FALSE;
    }

    application_state.game_inst = game_inst;

    application_state.width = application_state.game_inst->config.start_width;
    application_state.height = application_state.game_inst->config.start_height;

    // Initialize all subsystems
    log_startup();

    if (!platform_startup(
            &application_state.platform_state,
            game_inst->config.name,
            game_inst->config.start_pos_x,
            game_inst->config.start_pos_y,
            game_inst->config.start_width,
            game_inst->config.start_height)) { // Depends on: logger
        return FALSE;
    }

    memory_startup(); // Depends on: platform, logger

    if (!event_startup()) {
        ENGINE_ERROR("Failed to start event subsystem. Already initialized!");
        return FALSE;
    }

    input_startup(); // Depends on: logger, event, memory

    if (!renderer_startup(
            application_state.game_inst->config.name,
            &(application_state.platform_state))) {
        ENGINE_FATAL("Failed to initialize renderer frontend");
        return FALSE;
    }

    event_register_listener(
        Event_Code::APPLICATION_QUIT,
        nullptr,
        application_on_event);

    event_register_listener(
        Event_Code::KEY_PRESSED,
        nullptr,
        application_on_key);

    event_register_listener(
        Event_Code::KEY_RELEASED,
        nullptr,
        application_on_key);

    if (!application_state.game_inst->initialize(
            application_state.game_inst)) {
        ENGINE_ERROR("Failed to create game");
        return FALSE;
    }

    application_state.game_inst->on_resize(
        application_state.game_inst,
        application_state.width,
        application_state.height);

    application_state.is_running = TRUE;
    application_state.is_suspended = FALSE;

    is_initialized = TRUE;
    ENGINE_DEBUG("Subsystems initialized correctly.");

    ENGINE_DEBUG(memory_get_current_usage()); // WARN: Memory leak because the heap allocated string must be deallocated

    return TRUE;
}

void application_run() {
    application_state.is_running = TRUE;
    ENGINE_DEBUG("Application loop is starting...");

    absolute_clock_start(&application_state.clock);
    absolute_clock_update(&application_state.clock);

    f64 last_time = application_state.clock.elapsed_time;

    while (application_state.is_running) {
        // For each iteration read the new messages from the message queue
        if (!platform_message_pump(&application_state.platform_state)) {
            application_state.is_running = FALSE;
        }

        // Frame
        if (!application_state.is_suspended) {
            // To be consistent from the architecture standpoint the
            // clock will be updates once per frame
            absolute_clock_update(&application_state.clock);
            f64 current_time = application_state.clock.elapsed_time;
            f64 delta_t = current_time - last_time; // seconds

            // We need to compute the time needed to render an image
            f64 frame_start_time = platform_get_absolute_time();

            if (!application_state.game_inst->update(
                    application_state.game_inst,
                    static_cast<f32>(delta_t))) {
                ENGINE_FATAL("Game update failed. Aborting");
                application_state.is_running = FALSE;
                break;
            }

            if (!application_state.game_inst->render(
                    application_state.game_inst,
                    static_cast<f32>(delta_t))) {
                ENGINE_FATAL("Game render failed. Aborting");
                application_state.is_running = FALSE;
                break;
            }

            Render_Packet packet;
            packet.delta_time = delta_t;

            if (!renderer_draw_frame(&packet)) {
                application_state.is_running = FALSE;
            }

            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_time = frame_end_time - frame_start_time;

            f64 remaining_frame_seconds = TARGET_FRAME_TIME - frame_time;

            if (remaining_frame_seconds > 0) {
                b8 limit_frame = application_state.game_inst->config.limit_frame;
                // platform_sleep(milliseconds)
                if (limit_frame)
                    platform_sleep((remaining_frame_seconds * 1000) - 1);
            }

            // Input state copying should be the last thing
            input_update(delta_t);

            last_time = current_time;
        }
    }

    application_state.is_running = FALSE;

    absolute_clock_stop(&application_state.clock);
    application_shutdown();
}

void application_shutdown() {
    application_state.game_inst->shutdown(application_state.game_inst);
    // Start shutting down subsystems in reverse order to the startup order

    event_unregister_listener(
        Event_Code::APPLICATION_QUIT,
        nullptr,
        application_on_event);

    event_unregister_listener(
        Event_Code::KEY_PRESSED,
        nullptr,
        application_on_key);

    event_unregister_listener(
        Event_Code::KEY_RELEASED,
        nullptr,
        application_on_key);

    renderer_shutdown();
    input_shutdown();
    event_shutdown();
    memory_shutdown();
    platform_shutdown(&application_state.platform_state);
    log_shutdown();

    ENGINE_DEBUG("Application susbsytems stopped correctly");
}

b8 application_on_event(
    Event_Code code,
    void* sender,
    void* listener,
    Event_Context data) {
    switch (code) {
    case Event_Code::APPLICATION_QUIT:
        application_state.is_running = FALSE;
        return TRUE; // Stop other listeners from consuming the message
    default:
        // Propagate event to other listeners
        return FALSE;
    }
}

b8 application_on_key(
    Event_Code code,
    void* sender,
    void* listener,
    Event_Context data) {

    switch (code) {

    case Event_Code::KEY_PRESSED: {
        Keyboard_Key key = static_cast<Keyboard_Key>(data.data.u16[0]);
        u16 modifiers = data.data.u16[1];

        if (key == Keyboard_Key::ESCAPE) {
            event_fire(
                Event_Code::APPLICATION_QUIT,
                nullptr,
                data);

            return TRUE;

        } else if (key == Keyboard_Key::A) {
            ENGINE_INFO("Explicit key 'A' pressed");
        } else {
            ENGINE_INFO("Character '%c' pressed ", key);
        }
    } break;

    case Event_Code::KEY_RELEASED: {
        Keyboard_Key key = static_cast<Keyboard_Key>(data.data.u16[0]);

        if (key == Keyboard_Key::B)
            ENGINE_INFO("Explicit key 'B' released");
    } break;

    default:
        return FALSE;
    }

    return FALSE;
}
