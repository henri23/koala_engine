#pragma once

#include "defines.hpp"

// There will be no switch for fatal and error because they must always log
#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug loggin when releasing the application
#if KRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum log_level {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5,
} log_level;

b8 initialize_logging();
void shutdown_logging();

// The (...) means that the function can take a dynamic number of arguments
KAPI void log_output(log_level level, const char* message, ...);

// The __VA_ARGS__ is the way clang/gcc handles variable arguments
#define KFATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef KERROR
#define KERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define KWARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
#define KWARN(message, ...) // Do nothing when warning is turned off
#endif

#if LOG_TRACE_ENABLED == 1
#define KTRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else 
#define KTRACE(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define KINFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else 
#define KINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define KDEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else 
#define KDEBUG(message, ...)
#endif
