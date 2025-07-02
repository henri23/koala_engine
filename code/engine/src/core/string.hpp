#pragma once

#include "defines.hpp"
#include <stdarg.h>

KOALA_API b8 string_check_equal(
    const char* str1,
    const char* str2);

KOALA_API s32 string_format(
	char* dest, 
	const char* format, ...);

KOALA_API s32 string_format_v(
    char* dest,
    const char* format,
    va_list va_list);

KOALA_API u64 string_length(
    const char* string);
