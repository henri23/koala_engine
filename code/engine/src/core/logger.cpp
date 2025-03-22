#include "logger.hpp"

#include <stdarg.h>
#include <stdio.h>

#include "platform/platform.hpp"

b8 log_startup() {
    ENGINE_DEBUG("Loggin subsystem initialized");
    // TODO: create log file
    return TRUE;
}

void log_shutdown() {
    ENGINE_DEBUG("Loggin subsystem shutting down...");
    // TODO: cleanup logging/write queued entries
}

void log_output(log_scope scope, log_level level, const char* message, ...) {
    const char* level_strings[6] = {
        "[FATAL]: ",
        "[ERROR]: ",
        "[WARN]:  ",
        "[INFO]:  ",
        "[DEBUG]: ",
        "[TRACE]: "};

    const char* scope_strings[2] = {
        "ENGINE | ",
        "GAME   | "};

    b8 is_error = level < LOG_LEVEL_WARN;

    const s32 msg_length = 32000;
    char out_message[msg_length] = {};

    // NOTE:  MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some cases, and as a
    //        result throws a strange error here. The workaround for now is to just use __builtin_va_list, which is the
    //        type GCC/Clang's va_start expects
    __builtin_va_list arg_ptr;  // Gives us a pointer to the variable argument array

    va_start(arg_ptr, message);
    vsnprintf(out_message, msg_length, message, arg_ptr);  // Formats the message string with the variable arguments
    va_end(arg_ptr);

    char prepended_message[msg_length];
    // Prepend log level to the message string
    sprintf(prepended_message, "%s%s%s\n", scope_strings[scope], level_strings[level], out_message);

    // Platform specific output
    platform_console_write(prepended_message, level);
}

KOALA_API void report_assertion_failure(
    const char* expression,
    const char* message,
    const char* file,
    s32 line) {
    log_output(ASSERTS, LOG_LEVEL_FATAL, "Assertion failure: %s failed with message '%s', file %s, line %d\n", expression, message, file, line);
}
