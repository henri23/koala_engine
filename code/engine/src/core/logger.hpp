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

enum class Log_Level {
    FATAL = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4,
    TRACE = 5,
};

enum class Log_Scope {
    ENGINE = 0,
    GAME = 1,
    ASSERTS = 2
};

b8 log_startup();
void log_shutdown();

KOALA_API void log_output(Log_Scope scope, Log_Level level, const char* message, ...);

// The __VA_ARGS__ is the way clang/gcc handles variable arguments
#define ENGINE_FATAL(message, ...) log_output(Log_Scope::ENGINE, Log_Level::FATAL, message, ##__VA_ARGS__);
#define GAME_FATAL(message, ...) log_output(Log_Scope::GAME, Log_Level::FATAL, message, ##__VA_ARGS__);

#ifndef ENGINE_ERROR
#define ENGINE_ERROR(message, ...) log_output(Log_Scope::ENGINE, Log_Level::ERROR, message, ##__VA_ARGS__);
#endif

#ifndef GAME_ERROR
#define GAME_ERROR(message, ...) log_output(Log_Scope::GAME, Log_Level::ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define ENGINE_WARN(message, ...) log_output(Log_Scope::ENGINE, Log_Level::WARN, message, ##__VA_ARGS__);
#define GAME_WARN(message, ...) log_output(Log_Scope::GAME, Log_Level::WARN, message, ##__VA_ARGS__);
#else
#define ENGINE_WARN(message, ...) 
#define GAME_WARN(message, ...) 
#endif

#if LOG_TRACE_ENABLED == 1
#define ENGINE_TRACE(message, ...) log_output(Log_Scope::ENGINE, Log_Level::TRACE, message, ##__VA_ARGS__);
#define GAME_TRACE(message, ...) log_output(Log_Scope::GAME, Log_Level::TRACE, message, ##__VA_ARGS__);
#else 
#define ENGINE_TRACE(message, ...) 
#define GAME_TRACE(message, ...) 
#endif

#if LOG_INFO_ENABLED == 1
#define ENGINE_INFO(message, ...) log_output(Log_Scope::ENGINE, Log_Level::INFO, message, ##__VA_ARGS__);
#define GAME_INFO(message, ...) log_output(Log_Scope::GAME, Log_Level::INFO, message, ##__VA_ARGS__);
#else 
#define ENGINE_INFO(message, ...)
#define GAME_INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define ENGINE_DEBUG(message, ...) log_output(Log_Scope::ENGINE, Log_Level::DEBUG, message, ##__VA_ARGS__);
#define GAME_DEBUG(message, ...) log_output(Log_Scope::GAME, Log_Level::DEBUG, message, ##__VA_ARGS__);
#else 
#define ENGINE_DEBUG(message, ...) 
#define GAME_DEBUG(message, ...) 
#endif
