#pragma once

#include "defines.hpp"

// Memory lyaout : | 64 bits CAPACITY | 64 bits LENGTH | 64 bits STRIDE | void* variable size elements |
enum darray_header {
    DARRAY_CAPACITY,  // u64
    DARRAY_LENGTH,
    DARRAY_STRIDE,

    DARRAY_TOTAL_HEADER_LENGTH
};

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_EXPAND_FACTOR 2

// NOTE: We need to use a macro definition in order to extract the size of the type
#define darray_create(type) _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))
#define darray_reserve(type, capacity) _darray_create(capacity, sizeof(type))
KOALA_API void* _darray_create(u64 capacity, u64 stride);

KOALA_API void darray_destroy(void* array);

#define darray_length(array) _darray_field_get(array, DARRAY_LENGTH)
#define darray_stride(array) _darray_field_get(array, DARRAY_STRIDE)
#define darray_capacity(array) _darray_field_get(array, DARRAY_CAPACITY)
#define darray_clear(array) _darray_field_set(array, DARRAY_LENGTH, 0)
KOALA_API u64 _darray_field_get(void* array, u64 field);
KOALA_API void _darray_field_set(void* array, u64 field, u64 value);

#define darray_push(array, value)           \
    {                                       \
        auto temp = value;                  \
        array = _darray_push(array, &temp); \
    }
KOALA_API void* _darray_push(void* array, const void* value_ptr);
KOALA_API void darray_pop(void* array, void* dest);

#define darray_push_at(array, index, value)        \
    {                                              \
        auto temp = value;                         \
        array = _darray_insert_at(array, index, &temp); \
    }
KOALA_API void* _darray_insert_at(void* array, u32 index, const void* value_ptr);
KOALA_API void darray_pop_at(void* array, u32 index, void* dest);

KOALA_API void* _darray_resize(void* array);
