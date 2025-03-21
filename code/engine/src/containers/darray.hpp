#pragma once

#include "defines.hpp"

enum darray_header {
    LENGTH,
    SIZE,
    STRIDE,

    HEADER_SIZE
};

#define DARRAY_DEFAULT_SIZE 1
#define DARRAY_EXPAND_FACTOR 2

#define darray_create(type, length) _darray_create(sizeof(type), length))

void* _darray_create(u64 stride, u64 length);
