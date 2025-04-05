#pragma once

#include "defines.hpp"

// Memory lyaout : | 64 bits CAPACITY | 64 bits LENGTH | 64 bits STRIDE | void* variable size elements |
enum class Darray_Header : u64 {
    CAPACITY,  // u64
    LENGTH,
    STRIDE,

    FIELDS_MAX_COUNT
};

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_EXPAND_FACTOR 2

// NOTE: We need to use a macro definition in order to extract the size of the type
#define darray_create(type) _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))
#define darray_reserve(type, capacity) _darray_create(capacity, sizeof(type))
KOALA_API void* _darray_create(u64 capacity, u64 stride);

KOALA_API void darray_destroy(void* array);

#define darray_length(array) _darray_field_get(array, Darray_Header::LENGTH)
#define darray_stride(array) _darray_field_get(array, Darray_Header::STRIDE)
#define darray_capacity(array) _darray_field_get(array, Darray_Header::CAPACITY)
#define darray_clear(array) _darray_field_set(array, Darray_Header::HEADER_COUNT, 0)
KOALA_API u64 _darray_field_get(void* array, Darray_Header field);
KOALA_API void _darray_field_set(void* array, Darray_Header field, u64 value);

#define darray_push(array, value)           \
    {                                       \
        auto temp = value;                  \
        array = _darray_push(array, &temp); \
    }
KOALA_API void* _darray_push(void* array, const void* value_ptr);
KOALA_API void darray_pop(void* array, void* dest);

#define darray_insert_at(array, index, value)           \
    {                                                   \
        auto temp = value;                              \
        array = _darray_insert_at(array, index, &temp); \
    }
KOALA_API void* _darray_insert_at(void* array, u32 index, const void* value_ptr);
KOALA_API void darray_pop_at(void* array, u32 index, void* dest);

KOALA_API void* _darray_resize(void* array);
