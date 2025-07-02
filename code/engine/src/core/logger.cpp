#include "logger.hpp"

#include <stdarg.h>
#include <stdio.h>

#include "core/string.hpp"

#include "platform/filesystem.hpp"
#include "platform/platform.hpp"

// Create a dummy state, which will be used later, but for the time being it is
// just a showcase to use the linear allocator

struct Logger_System_State {
    File_Handle log_file_handle;
};

internal Logger_System_State* state_ptr;

void append_to_log_file(const char* message) {
    if (state_ptr && state_ptr->log_file_handle.is_valid) {
        // The message already contains the character \n because it is
        // formatted
        u64 length = string_length(message);
        u64 written = 0;
        if (!filesystem_write(
                &state_ptr->log_file_handle,
                length,
                message,
                &written)) {
            platform_console_write_error(
                "ERROR writing to console.log",
                static_cast<u8>(Log_Level::ERROR));
        }
    }
}

// Use a Vulkan pattern for system initialization where we call each systems
// init twice, one to retrieve the memory requirement and the second time to
// pass the allocated memory
b8 log_startup(u64* memory_requirement, void* state) {

    *memory_requirement = sizeof(Logger_System_State);

    if (state == nullptr) {
        return true;
    }

    state_ptr = static_cast<Logger_System_State*>(state);

    ENGINE_DEBUG("Loggin subsystem initialized");

    // Create log file or wipe an existing one
    if (!filesystem_open(
            "console.log",
            File_Modes::WRITE,
            false,
            &state_ptr->log_file_handle)) {

        platform_console_write_error(
            "ERROR: Unable to open console.log for writing",
            static_cast<u8>(Log_Level::ERROR));

        return false;
    }

    return true;
}

void log_shutdown(void* state) {
    state_ptr = nullptr;

    ENGINE_DEBUG("Loggin subsystem shutting down...");
    // TODO: cleanup logging/write queued entries
}

// TODO: String operations are generally very slow.
// This must be moved to a separate thread ideally
void log_output(Log_Scope scope, Log_Level level, const char* message, ...) {
    const char* level_strings[6] = {
        "[FATAL]: ",
        "[ERROR]: ",
        "[WARN]:  ",
        "[INFO]:  ",
        "[DEBUG]: ",
        "[TRACE]: "};

    const char* scope_strings[3] = {
        "ENGINE | ",
        "GAME   | ",
        "ASSERT | "};

    b8 is_error = (u64)level < (u64)Log_Level::WARN;

    char out_message[32000] = {};

    // NOTE:  MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some cases, and as a
    //        result throws a strange error here. The workaround for now is to just use __builtin_va_list, which is the
    //        type GCC/Clang's va_start expects
    va_list arg_ptr; // Pointer to args

    va_start(arg_ptr, message);
    string_format_v(out_message, message, arg_ptr);
    va_end(arg_ptr);

    // Prepend log level to the message string
    string_format(
        out_message,
        "%s%s%s\n",
        scope_strings[static_cast<u64>(scope)],
        level_strings[static_cast<u64>(level)],
        out_message);

    if (is_error)
        platform_console_write_error(out_message, static_cast<u64>(level));
    else
        // Platform specific output
        platform_console_write(out_message, static_cast<u64>(level));

    append_to_log_file(out_message);
}

KOALA_API void report_assertion_failure(
    const char* expression,
    const char* message,
    const char* file,
    s32 line) {
    log_output(
        Log_Scope::ASSERTS,
        Log_Level::FATAL,
        "Assertion failure: %s failed with message '%s', file %s, line %d\n",
        expression, message, file, line);
}
