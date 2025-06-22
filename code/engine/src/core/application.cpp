#include "application.hpp"

#include "core/absolute_clock.hpp"
#include "core/event.hpp"
#include "core/input.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "game_types.hpp"

#include "platform/platform.hpp"

#include "memory/linear_allocator.hpp"

#include "renderer/renderer_frontend.hpp"
#include "renderer/renderer_types.inl"

constexpr f64 TARGET_FRAME_TIME = 1.0f / 60;

struct Application_State {
    Game* game_inst;

    b8 is_running;
    b8 is_suspended;

    s16 width;
    s16 height;

    Absolute_Clock clock;
    Linear_Allocator systems_allocator;

    u64 logging_system_mem_req;
    void* logging_system_state;

    u64 memory_system_mem_req;
    void* memory_system_state;

    u64 event_system_mem_req;
    void* event_system_state;

    u64 input_system_mem_req;
    void* input_system_state;

    u64 platform_system_mem_req;
    void* platform_system_state;

    u64 renderer_system_mem_req;
    void* renderer_system_state;
};

// Store a pointer for easy access to the application state, since the app state
// will be stored in the game side, inside the game instance
internal Application_State* application_state;

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

b8 application_on_resized(
    Event_Code code,
    void* sender,
    void* listener,
    Event_Context data);

b8 application_initialize(Game* game_inst) {
    // TODO: Automate process of susbsytem initialization in a queue with dependency order optimization

    // Protect the application state from being initialized multiple times
    if (game_inst->application_state != nullptr) {
        ENGINE_ERROR("Engine subsystems are already initialized");
        return FALSE;
    }

    game_inst->application_state = memory_allocate(
        sizeof(Application_State),
        Memory_Tag::APPLICATION);

    application_state = static_cast<Application_State*>(
        game_inst->application_state);
    application_state->game_inst = game_inst;

    application_state->is_running = FALSE;
    application_state->is_suspended = FALSE;

    // Setup linear allocator
    u64 systems_alloc_total_size = 64 * 1024 * 1024; // For now just set to a large number
    linear_allocator_create(
        systems_alloc_total_size,
        nullptr,
        &application_state->systems_allocator);

    application_state->width =
        application_state->game_inst->config.start_width;

    application_state->height =
        application_state->game_inst->config.start_height;

    // Initialize all subsystems

    // 1. Logging subsystem
    log_startup(&application_state->logging_system_mem_req, nullptr);
    application_state->logging_system_state = linear_allocator_allocate(
        &application_state->systems_allocator,
        application_state->logging_system_mem_req);
    if (!log_startup(
            &application_state->logging_system_mem_req,
            application_state->logging_system_state)) {
        ENGINE_ERROR("Failed to initialize logging subsystem; shutting down");
        return FALSE;
    }

    // 2. Platform layer - Depends on: logger
    platform_startup(
        &application_state->platform_system_mem_req,
        nullptr,
        game_inst->config.name,
        game_inst->config.start_pos_x,
        game_inst->config.start_pos_y,
        game_inst->config.start_width,
        game_inst->config.start_height);

    application_state->platform_system_state = linear_allocator_allocate(
        &application_state->systems_allocator,
        application_state->platform_system_mem_req);

    if (!platform_startup(
            &application_state->platform_system_mem_req,
            application_state->platform_system_state,
            game_inst->config.name,
            game_inst->config.start_pos_x,
            game_inst->config.start_pos_y,
            game_inst->config.start_width,
            game_inst->config.start_height)) {

        ENGINE_ERROR("Failed to initialize platform. Aborting...");
        return FALSE;
    }

    // 3. Memory subsystem
    memory_startup(&application_state->memory_system_mem_req, nullptr);
    application_state->memory_system_state = linear_allocator_allocate(
        &application_state->systems_allocator,
        application_state->memory_system_mem_req);
    memory_startup(
        &application_state->memory_system_mem_req,
        application_state->memory_system_state);

    // 4. Event subsystem
    event_startup(&application_state->event_system_mem_req, nullptr);
    application_state->event_system_state = linear_allocator_allocate(
        &application_state->systems_allocator,
        application_state->event_system_mem_req);
    if (!event_startup(
            &application_state->event_system_mem_req,
            application_state->event_system_state)) {
        ENGINE_ERROR("Failed to start event subsystem. Already initialized!");
        return FALSE;
    }

    // 5. Input subsystem - Depends on: logger, event, memory
    input_startup(&application_state->input_system_mem_req, nullptr);
    application_state->input_system_state = linear_allocator_allocate(
        &application_state->systems_allocator,
        application_state->input_system_mem_req);
    input_startup(
        &application_state->input_system_mem_req,
        application_state->input_system_state);

    // 6. Renderer startup (Call frontend but implicitly starting backend)
    renderer_startup(
        &application_state->renderer_system_mem_req,
        nullptr,
        application_state->game_inst->config.name);

    application_state->renderer_system_state = linear_allocator_allocate(
        &application_state->systems_allocator,
        application_state->renderer_system_mem_req);

    if (!renderer_startup(
            &application_state->renderer_system_mem_req,
            application_state->renderer_system_state,
            application_state->game_inst->config.name)) {

        ENGINE_FATAL("Failed to initialize renderer frontend");
        return FALSE;
    }

    // Event listener registration
    event_register_listener(
        Event_Code::RESIZED,
        nullptr,
        application_on_resized);

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

    if (!application_state->game_inst->initialize(
            application_state->game_inst)) {
        ENGINE_ERROR("Failed to create game");
        return FALSE;
    }

    application_state->game_inst->on_resize(
        application_state->game_inst,
        application_state->width,
        application_state->height);

    application_state->is_suspended = FALSE;

    ENGINE_DEBUG("Subsystems initialized correctly.");

    ENGINE_DEBUG(memory_get_current_usage()); // WARN: Memory leak because the heap allocated string must be deallocated

    return TRUE;
}

void application_run() {
    application_state->is_running = TRUE;
    ENGINE_DEBUG("Application loop is starting...");

    absolute_clock_start(&application_state->clock);
    absolute_clock_update(&application_state->clock);

    f64 last_time = application_state->clock.elapsed_time;

    while (application_state->is_running) {
        // For each iteration read the new messages from the message queue
        if (!platform_message_pump()) {
            application_state->is_running = FALSE;
        }

        // Frame
        if (!application_state->is_suspended) {
            // To be consistent from the architecture standpoint the
            // clock will be updates once per frame
            absolute_clock_update(&application_state->clock);
            f64 current_time = application_state->clock.elapsed_time;
            f64 delta_t = current_time - last_time; // seconds

            // We need to compute the time needed to render an image
            f64 frame_start_time = platform_get_absolute_time();

            if (!application_state->game_inst->update(
                    application_state->game_inst,
                    static_cast<f32>(delta_t))) {
                ENGINE_FATAL("Game update failed. Aborting");
                application_state->is_running = FALSE;
                break;
            }

            if (!application_state->game_inst->render(
                    application_state->game_inst,
                    static_cast<f32>(delta_t))) {
                ENGINE_FATAL("Game render failed. Aborting");
                application_state->is_running = FALSE;
                break;
            }

            Render_Packet packet;
            packet.delta_time = delta_t;

            if (!renderer_draw_frame(&packet)) {
                application_state->is_running = FALSE;
            }

            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_time = frame_end_time - frame_start_time;

            f64 remaining_frame_seconds = TARGET_FRAME_TIME - frame_time;

            if (remaining_frame_seconds > 0) {
                b8 limit_frame =
                    application_state->game_inst->config.limit_frame;

                // platform_sleep(milliseconds)
                if (limit_frame)
                    platform_sleep((remaining_frame_seconds * 1000) - 1);
            }

            // Input state copying should be the last thing
            input_update(delta_t);

            last_time = current_time;
        }
    }

    application_state->is_running = FALSE;

    absolute_clock_stop(&application_state->clock);
    application_shutdown();
}

void application_get_framebuffer_size(u32* width, u32* height) {
    *width = application_state->width;
    *width = application_state->width;
}

void application_shutdown() {
    application_state->game_inst->shutdown(application_state->game_inst);
    // Start shutting down subsystems in reverse order to the startup order

    event_unregister_listener(
        Event_Code::RESIZED,
        nullptr,
        application_on_resized);

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

    renderer_shutdown(application_state->renderer_system_state);
    input_shutdown(application_state->input_system_state);
    event_shutdown(application_state->event_system_state);
    memory_shutdown(application_state->memory_system_state);
    platform_shutdown(application_state->platform_system_state);
    log_shutdown(application_state->logging_system_state);

    ENGINE_DEBUG("Application susbsytems stopped correctly");
}

b8 application_on_event(
    Event_Code code,
    void* sender,
    void* listener,
    Event_Context data) {
    switch (code) {
    case Event_Code::APPLICATION_QUIT:
        application_state->is_running = FALSE;
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

b8 application_on_resized(
    Event_Code code,
    void* sender,
    void* listener,
    Event_Context data) {

    if (code == Event_Code::RESIZED) {
        u16 width = data.data.u16[0];
        u16 height = data.data.u16[1];

        if (width != application_state->width ||
            height != application_state->height) {

            application_state->width = width;
            application_state->height = height;

            ENGINE_DEBUG("Windows resize: %i, %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0) {
                ENGINE_INFO("Windows minimized, suspending application.");
                application_state->is_suspended = TRUE;
                return TRUE;
            } else {
                if (application_state->is_suspended) {
                    ENGINE_INFO("Window restored, resuming application");
                    application_state->is_suspended = FALSE;
                }

                application_state->game_inst->on_resize(
                    application_state->game_inst,
                    width,
                    height);

                renderer_on_resize(width, height);
            }
        }
    }

    return TRUE;
}
