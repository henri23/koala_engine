#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long s64;

typedef float f32;
typedef double f64;

typedef int b32;
typedef char b8;

// Properly define static assertions
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT static_assert
#else
#define STATIC_ASSERT Static_assert
#endif

STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes");

STATIC_ASSERT(sizeof(s8) == 1, "Expected s8 to be 1 byte");
STATIC_ASSERT(sizeof(s16) == 2, "Expected s16 to be 2 bytes");
STATIC_ASSERT(sizeof(s32) == 4, "Expected s32 to be 4 bytes");
STATIC_ASSERT(sizeof(s64) == 8, "Expected s64 to be 8 bytes");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes");

#define TRUE 1
#define FALSE 0

#define GIB ((u64)1 << 30)
#define MIB ((u64)1 << 20)
#define KIB ((u64)1 << 10)

#define local_persist static
#define internal static
#define global_variable static

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define ENGINE_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define ENGINE_PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define ENGINE_PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define ENGINE_PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define ENGINE_PLATFORM_POSIX 1
#elif __APPLE__
// Apple platforms
#define ENGINE_PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define ENGINE_PLATFORM_IOS 1
#define ENGINE_PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define ENGINE_PLATFORM_IOS 1
// iOS device
#elif TARGET_OS_MAC
// Other kinds of macOS
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform!"
#endif

/*
  When building a .dll we want to export the functions that we want to make available from the dll
*/
#ifdef KOALA_EXPORT

// Exports
#ifdef _MSC_VER
#define KOALA_API __declspec(dllexport)
#else
#define KOALA_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define KOALA_API __declspec(dllimport)
#else
#define KOALA_API
#endif
#endif
#define SKIP_ASAN __attribute__((no_sanitize("address")))
