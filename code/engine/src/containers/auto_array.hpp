#pragma once

#include "core/asserts.hpp"
#include "core/memory.hpp"

#include "defines.hpp"

#define AUTO_DARRAY_DEFEAULT_CAPACITY 1
#define AUTO_DARRAY_EXPAND_FACTOR 2

template <typename T>
struct KOALA_API Auto_Array {
    T* data;
    u64 capacity;
    u64 length;

    Auto_Array<T>() {
        data = nullptr;
        capacity = 0;
        length = 0;
    };

    void add(const T& value) {
        if (length >= capacity)
            grow();

        memory_copy(
            data + length,
            &value,
            sizeof(T));

        ++length;
    }

    void reserve(u64 data_count) {
        capacity = data_count;

        data = static_cast<T*>(
            memory_allocate(
                sizeof(T) * data_count,
                Memory_Tag::DARRAY));

		length = data_count;
    }

    void free() {
        memory_deallocate(
            data,
            sizeof(T) * length,
            Memory_Tag::DARRAY);

        length = 0;
        capacity = 0;
        data = nullptr;
    }

    void clear() { length = 0; }

    void grow() {
        u64 new_capacity = capacity != 0
                               ? capacity * AUTO_DARRAY_EXPAND_FACTOR
                               : AUTO_DARRAY_DEFEAULT_CAPACITY;

        void* new_array = memory_allocate(
            new_capacity * sizeof(T),
            Memory_Tag::DARRAY);

        if (length > 0)
            memory_copy(
                new_array,
                data,
                sizeof(T) * length);

        memory_deallocate(
            data,
            sizeof(T) * length,
            Memory_Tag::DARRAY);

        capacity = new_capacity;
        data = static_cast<T*>(new_array);
    }

    void pop() {
        --length;
    }

    void pop_at(u32 index) {
        RUNTIME_ASSERT(index <= length);

        if (index != length - 1)
            memory_copy(
                data + index,
                data + index + 1,
                sizeof(T) * (length - (index - 1)));

        --length;
    }

    T& operator[](u32 index) {
        RUNTIME_ASSERT(index < length);

        return data[index];
    }
};
