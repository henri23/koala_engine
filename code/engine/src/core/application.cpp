#include "application.hpp"

#include "core/event.hpp"
#include "core/input.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "game_types.hpp"
#include "platform/platform.hpp"

struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform_state;
    s16 width;
    s16 height;
};

internal b8 is_initialized = FALSE;
internal application_state application_state;

// Forward declared event handlers. Implemented below
b8 application_on_event(
    event_code code,
    void* sender,
    void* listener,
    event_context data);

b8 application_on_key(
    event_code code,
    void* sender,
    void* listener,
    event_context data);

b8 application_initialize(game* game_inst) {
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
            game_inst->config.start_height)) {  // Depends on: logger
        return FALSE;
    }

    memory_startup();  // Depends on: platform, logger

    if (!event_startup()) {
        ENGINE_ERROR("Failed to start event subsystem. Already initialized!");
        return FALSE;
    }

    input_startup();  // Depends on: logger, event, memory

    event_register_listener(
        event_code::APPLICATION_QUIT,
        nullptr,
        application_on_event);

    event_register_listener(
        event_code::KEY_PRESSED,
        nullptr,
        application_on_key);

    event_register_listener(
        event_code::KEY_RELEASED,
        nullptr,
        application_on_key);

    if (!application_state.game_inst->initialize(application_state.game_inst)) {
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

    ENGINE_DEBUG(memory_get_current_usage());  // WARN: Memory leak because the heap allocated string must be deallocated

    return TRUE;
}

void application_run() {
    application_state.is_running = TRUE;
    ENGINE_DEBUG("Application loop is starting...");

    while (application_state.is_running) {
        // For each iteration read the new messages from the message queue
        if (!platform_message_pump(&application_state.platform_state)) {
            application_state.is_running = FALSE;
        }

        // Frame
        if (!application_state.is_suspended) {
            if (!application_state.game_inst->update(
                    application_state.game_inst,
                    0.0f)) {
                ENGINE_FATAL("Game update failed. Aborting");
                application_state.is_running = FALSE;
                break;
            }

            if (!application_state.game_inst->render(
                    application_state.game_inst,
                    0.0f)) {
                ENGINE_FATAL("Game render failed. Aborting");
                application_state.is_running = FALSE;
                break;
            }

            // NOTE: Input state copying should be the last thing done in a frame
            input_update(0);
        }
    }

    application_state.is_running = FALSE;

    application_shutdown();
}

void application_shutdown() {
    application_state.game_inst->shutdown(application_state.game_inst);
    // Start shutting down subsystems in reverse order to the startup order

    event_unregister_listener(
        event_code::APPLICATION_QUIT,
        nullptr,
        application_on_event);

    event_unregister_listener(
        event_code::KEY_PRESSED,
        nullptr,
        application_on_key);

    event_unregister_listener(
        event_code::KEY_RELEASED,
        nullptr,
        application_on_key);

    input_shutdown();
    event_shutdown();
    memory_shutdown();
    platform_shutdown(&application_state.platform_state);
    log_shutdown();

    ENGINE_DEBUG("Application susbsytems stopped correctly");
}

b8 application_on_event(
    event_code code,
    void* sender,
    void* listener,
    event_context data) {
    switch (code) {
        case event_code::APPLICATION_QUIT:
            application_state.is_running = FALSE;
            return TRUE;  // Stop other listeners from consuming the message
        default:
            // Propagate event to other listeners
            return FALSE;
    }
}

b8 application_on_key(
    event_code code,
    void* sender,
    void* listener,
    event_context data) {
    switch (code) {
        case event_code::KEY_PRESSED: {
            keyboard_key key = static_cast<keyboard_key>(data.data.u16[0]);
            u16 modifiers = data.data.u16[1];

            if (key == keyboard_key::ESCAPE) {
                event_fire(
                    event_code::APPLICATION_QUIT,
                    nullptr,
                    data);

                return TRUE;

            } else if (key == keyboard_key::A) {
                ENGINE_INFO("Explicit key 'A' pressed");
            } else {
                ENGINE_INFO("Character '%c' pressed ", key);
            }
        } break;
        case event_code::KEY_RELEASED: {
            keyboard_key key = static_cast<keyboard_key>(data.data.u16[0]);

            if (key == keyboard_key::B)
                ENGINE_INFO("Explicit key 'B' released");
        } break;
        default:
            return FALSE;
    }

    return FALSE;
}
