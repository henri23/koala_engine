#pragma once

#include "defines.hpp"

struct Linear_Allocator {
    u64 total_size;
    u64 allocated;
    void* memory;
    // Mark whether the memory is owned by the allocator, so that if the memory
    // is owned by the allocator, it should be freed when the allocator is
    // destroyed
    b8 owns_memory;
};

KOALA_API void linear_allocator_create(
    u64 total_size,
    void* memory,
    Linear_Allocator* out_allocator);

KOALA_API void linear_allocator_destroy(
    Linear_Allocator* allocator);

KOALA_API void* linear_allocator_allocate(
    Linear_Allocator* allocator,
    u64 size);

KOALA_API void linear_allocator_free_all(
    Linear_Allocator* allocator);
