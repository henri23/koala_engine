#include "linear_allocator.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

void linear_allocator_create(
    u64 total_size,
    void* memory,
    Linear_Allocator* out_allocator) {

    if (out_allocator) {
        out_allocator->total_size = total_size;
        out_allocator->allocated = 0;
        out_allocator->owns_memory = memory == nullptr;
        if (memory)
            out_allocator->memory = memory;
        else
            out_allocator->memory = memory_allocate(
                total_size,
                Memory_Tag::LINEAR_ALLOCATOR);
    }
}

void linear_allocator_destroy(
    Linear_Allocator* allocator) {
    if (allocator) {
        allocator->allocated = 0;
        if (allocator->owns_memory && allocator->memory) {
            memory_deallocate(
                allocator->memory,
                allocator->total_size,
                Memory_Tag::LINEAR_ALLOCATOR);
        }

        allocator->memory = nullptr;
        allocator->total_size = 0;
        allocator->owns_memory = false;
    }
}

void* linear_allocator_allocate(
    Linear_Allocator* allocator,
    u64 size) {
    if (allocator && allocator->memory) {
        if (allocator->allocated + size > allocator->total_size) {
            u64 remaining = allocator->total_size - allocator->allocated;
            ENGINE_ERROR("linear_allocator_allocate - Tried to allocate %llu but only %llu bytes are available", size, remaining);
            return nullptr;
        }

        void* block =
            static_cast<u8*>(allocator->memory) + allocator->allocated;

        allocator->allocated += size;
        return block;
    }

    ENGINE_ERROR("linear_allocator_allocate - allocator not initialized");
    return nullptr;
}

void linear_allocator_free_all(
    Linear_Allocator* allocator) {

    if (allocator && allocator->memory) {
        allocator->allocated = 0;
        memory_zero(allocator->memory, allocator->total_size);
    }

    ENGINE_ERROR("linear_allocator_free_all - allocator not initialized");
}
