#include "memory.hpp"

#include <stdio.h>
#include <string.h>

#include "core/logger.hpp"
#include "defines.hpp"
#include "platform/platform.hpp"

struct memory_stats {
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_ENTRIES];
};

internal memory_stats stats;

internal const char* memory_tag_strings[MEMORY_TAG_MAX_ENTRIES] = {
    "UNKNOWN  :",
    "DARRAY   :",
    "STRING   :",
    "EVENTS   :"};

void memory_startup() {
    platform_zero_memory(&stats, sizeof(stats));
    ENGINE_DEBUG("Memory subsystem initialized");
}

void memory_shutdown() {
    ENGINE_DEBUG("Memory subsystem shutting down...");
}

KOALA_API void* memory_allocate(u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        ENGINE_WARN("The memory is being initialized as UNKNOWN. Please allocated it with the proper tag");
    }

    stats.tagged_allocations[tag] += size;
    stats.total_allocated += size;

    // Every chunk of memory will be set to 0 automatically
    void* block = platform_allocate(size, TRUE);

    platform_zero_memory(block, size);

    return block;
}

KOALA_API void memory_deallocate(void* block, u64 size, memory_tag tag) {
    stats.tagged_allocations[tag] -= size;
    stats.total_allocated -= size;

    return platform_free(block, TRUE);
}

KOALA_API void* memory_zero(void* block, u64 size) {
    return platform_zero_memory(block, size);
}

KOALA_API void* memory_copy(void* destination, const void* source, u64 size) {
    return platform_copy_memory(destination, source, size);
}

KOALA_API void* memory_move(void* destination, const void* source, u64 size) {
    return platform_move_memory(destination, source, size);
}

KOALA_API void* memory_set(void* block, s32 value, u64 size) {
    return platform_set_memory(block, value, size);
}

KOALA_API char* memory_get_current_usage() {

    char utilization_buffer[5000] = "Summary of allocated memory (tagged):\n";

    u64 offset = strlen(utilization_buffer);  // The offset is represented in number of bytes

    for (u32 i = 0; i < MEMORY_TAG_MAX_ENTRIES; ++i) {
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
    char* copy = static_cast<char *>(memory_allocate(length + 1, MEMORY_TAG_STRING));
    memory_copy(copy, utilization_buffer, length + 1);

    return copy;
}
