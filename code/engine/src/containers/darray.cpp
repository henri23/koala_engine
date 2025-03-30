#include "darray.hpp"

#include "core/logger.hpp"
#include "core/memory.hpp"

void* _darray_create(u64 capacity, u64 stride) {
    u64 array_length = capacity * stride;
    array_length += (u64)darray_header::FIELDS_MAX_COUNT * sizeof(u64);  // Need to consider also the header when allocating memory for the array
    u64* header = static_cast<u64*>(
        memory_allocate(
            array_length,
            memory_tag::DARRAY));

    header[(u64)darray_header::CAPACITY] = capacity;
    header[(u64)darray_header::LENGTH] = 0;
    header[(u64)darray_header::STRIDE] = stride;
    return static_cast<void*>(header + (u64)darray_header::FIELDS_MAX_COUNT);  // Return the pointer pointing to the 0 element of the array
}

void darray_destroy(void* array) {
    u64* header = static_cast<u64*>(array) - (u64)darray_header::FIELDS_MAX_COUNT;  // Move back to the start of the header
    u64 array_size = header[(u64)darray_header::CAPACITY] * header[(u64)darray_header::STRIDE];
    u64 memory_size = array_size + (u64)darray_header::FIELDS_MAX_COUNT * sizeof(u64);
    memory_deallocate(header, memory_size, memory_tag::DARRAY);
}

u64 _darray_field_get(void* array, darray_header field) {
    u64* header = static_cast<u64*>(array) - static_cast<u64>(darray_header::FIELDS_MAX_COUNT);
    return header[(u64)field];
}

void _darray_field_set(void* array, darray_header field, u64 value) {
    u64* header = static_cast<u64*>(array) - static_cast<u64>(darray_header::FIELDS_MAX_COUNT);
    header[(u64)field] = value;
}

void* _darray_resize(void* array) {
    u64 stride = darray_stride(array);
    u64 length = darray_length(array);

    // Allocate space for the expanded new array will proper capacity and stride but empty length
    void* new_array = _darray_create(
        darray_capacity(array) * DARRAY_EXPAND_FACTOR,
        stride);

    memory_copy(new_array, array, stride * length);

    _darray_field_set(new_array, darray_header::LENGTH, length);
    darray_destroy(array);

    return new_array;
}

void* _darray_push(void* array, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    if (length >= darray_capacity(array))
        array = _darray_resize(array);

    // NOTE:  By casing to u8 each addition will move the pointer by 1 byte
    //        Since stride is the number of bytes of an element of the array
    //        by adding a stride to the mapped pointer we will move exactly
    //        to next element. By multiplying with the length we do the desired location
    u8* addr = static_cast<u8*>(array);
    addr += length * stride;
    memory_copy(addr, value_ptr, stride);
    _darray_field_set(array, darray_header::LENGTH, length + 1);
    return array;
}

void darray_pop(void* array, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    u8* index = static_cast<u8*>(array) + (length - 1) * stride;
    memory_copy(dest, index, stride);
    _darray_field_set(array, darray_header::LENGTH, length - 1);
}

void* _darray_insert_at(void* array, u32 index, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    // Index always starts from 0
    if (index >= length) {
        ENGINE_ERROR("Index is out of the boundaries of the array");
        return array;
    }

    if (length >= darray_capacity(array))
        array = _darray_resize(array);

    u8* address = static_cast<u8*>(array);

    // If index is the last index there is nothing to copy, however if the index is an occupied index
    // we need to copy all the elements from the index until the end to the next slots adjacent
    if (index != length - 1)
        // Since the two memory regions overlap, we use memmove instead of memcpy because it is safer
        memory_copy(
            (address + (index + 1) * stride),
            (address + index * stride),
            (length - index) * stride);

    memory_copy(
        (address + index * stride),
        value_ptr,
        stride);

    _darray_field_set(array, darray_header::LENGTH, length + 1);
    return array;
}

void darray_pop_at(void* array, u32 index, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    // Index always starts from 0
    if (index >= length)
        ENGINE_ERROR("Index is out of the boundaries of the array");

    u8* address = static_cast<u8*>(array);

    // First copy the element at the index to the destination address
    memory_copy(
        dest,
        (address + index * stride),
        stride);

    if (index != length - 1)
        memory_copy(
            (address + index * stride),
            (address + (index + 1) * stride),
            (length - index - 1) * stride);

    _darray_field_set(array, darray_header::LENGTH, length - 1);
}
