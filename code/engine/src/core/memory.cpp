#include "memory.hpp"

#include <stdio.h>
#include <string.h>

#include "core/logger.hpp"
#include "defines.hpp"
#include "platform/platform.hpp"

struct Memory_Stats {
    u64 total_allocated;
    u64 tagged_allocations[(u64)Memory_Tag::MAX_ENTRIES];
};

internal Memory_Stats stats;

internal const char* memory_tag_strings[(u64)Memory_Tag::MAX_ENTRIES] = {
    "UNKNOWN  :",
    "DARRAY   :",
    "EVENTS   :",
    "STRING   :",
    "GAME     :",
    "INPUT    :",
};

void memory_initialize() {
    // We do not zero when startup is called because the memory is available
    // before the application is started
    // platform_zero_memory(&stats, sizeof(stats));
    ENGINE_DEBUG("Memory subsystem initialized");
}

void memory_shutdown() {
    ENGINE_DEBUG("Memory subsystem shutting down...");
}

void* memory_allocate(u64 size, Memory_Tag tag) {
    if (tag == Memory_Tag::UNKNOWN) {
        ENGINE_WARN("The memory is being initialized as UNKNOWN. Please allocated it with the proper tag");
    }

    stats.tagged_allocations[(u64)tag] += size;
    stats.total_allocated += size;

    // Every chunk of memory will be set to 0 automatically
    void* block = platform_allocate(size, TRUE);

    platform_zero_memory(block, size);

    return block;
}

void memory_deallocate(void* block, u64 size, Memory_Tag tag) {
    stats.tagged_allocations[(u64)tag] -= size;
    stats.total_allocated -= size;

    return platform_free(block, TRUE);
}

void* memory_zero(void* block, u64 size) {
    return platform_zero_memory(block, size);
}

void* memory_copy(void* destination, const void* source, u64 size) {
    b8 is_overlap = FALSE;

    u64 dest_addr = reinterpret_cast<u64>(destination);
    u64 source_addr = reinterpret_cast<u64>(source);

    if (source_addr == dest_addr) {
        ENGINE_WARN("Method memory_copy() called with identical source and destination addresses. No action will occur");
        return destination;
    }

    // Since we are adding the size to the integer value of the address, the compiler must
    // copy 'size' bytes to the destination. When copying 'size' bytes, the address value
    // will be incremented by ('size' - 1). So if the max_addr coincides with min_addr + size - 1
    // there will be overlap.
    if (source_addr > dest_addr && source_addr < dest_addr + size)
        is_overlap = TRUE;
    else if (dest_addr > source_addr && dest_addr < source_addr + size)
        is_overlap = TRUE;

    if (!is_overlap)
        return platform_copy_memory(destination, source, size);
    else {
        ENGINE_DEBUG("Method memory_copy() called with overlapping regions of memory, using memmove() instead");
        return platform_move_memory(destination, source, size);
    }
}

void* memory_move(void* destination, const void* source, u64 size) {
    return platform_move_memory(destination, source, size);
}

void* memory_set(void* block, s32 value, u64 size) {
    return platform_set_memory(block, value, size);
}

char* memory_get_current_usage() {
    char utilization_buffer[5000] = "Summary of allocated memory (tagged):\n";

    u64 offset = strlen(utilization_buffer);  // The offset is represented in number of bytes

    u64 max_tags = static_cast<u64>(Memory_Tag::MAX_ENTRIES);
    for (u32 i = 0; i < max_tags; ++i) {
        char usage_unit[4] = "XiB";
        f32 amount = 1.0f;

        if (stats.tagged_allocations[i] >= GIB) {
            usage_unit[0] = 'G';
            amount = (float)stats.tagged_allocations[i] / GIB;
        } else if (stats.tagged_allocations[i] >= MIB) {
            usage_unit[0] = 'M';
            amount = (float)stats.tagged_allocations[i] / MIB;
        } else if (stats.tagged_allocations[i] >= KIB) {
            usage_unit[0] = 'K';
            amount = (float)stats.tagged_allocations[i] / KIB;
        } else {
            usage_unit[0] = 'B';
            usage_unit[1] = 0;  // Append a null termination character to overwrite the end of the string
            amount = (float)stats.tagged_allocations[i];
        }

        // snprintf returns the number of writen characters (aka bytes). It returns negative number if there was an error
        s32 length = snprintf(
            utilization_buffer + offset,
            sizeof(utilization_buffer) - offset,
            "%s %.2f %s\n", memory_tag_strings[i], amount, usage_unit);

        offset += length;
    }

    // In order to use this buffer to another place we need to copy its value into a dynamically allocated memory
    // This is because the buffer will go out of scope after we return and the value will be jibrish
    // We need one more byte for the null terminator character as the strlen disregards it
    u64 length = strlen(utilization_buffer);
    char* copy = static_cast<char*>(memory_allocate(length + 1, Memory_Tag::STRING));
    memory_copy(copy, utilization_buffer, length + 1);

    return copy;
}
