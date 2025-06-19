#include "linear_allocator_tests.hpp"
#include "../expect.hpp"
#include "../test_manager.hpp"
#include <memory/linear_allocator.hpp>

u8 linear_allocator_should_create_and_destroy() {
    Linear_Allocator alloc;
    linear_allocator_create(sizeof(u64), 0, &alloc);

    expect_should_not_be(0, alloc.memory);
    expect_should_be(sizeof(u64), alloc.total_size);
    expect_should_be(0, alloc.allocated);

    linear_allocator_destroy(&alloc);

    expect_should_be(0, alloc.memory);
    expect_should_be(0, alloc.total_size);
    expect_should_be(0, alloc.allocated);

    return true;
}

u8 linear_allocator_allocate_all_space_single() {
    Linear_Allocator alloc;

    linear_allocator_create(
        sizeof(u64),
        nullptr,
        &alloc);

    void* block = linear_allocator_allocate(&alloc, sizeof(u64));

    expect_should_not_be(nullptr, alloc.memory);
	expect_should_be(sizeof(u64), alloc.allocated);

    linear_allocator_destroy(&alloc);

    return true;
}
u8 linear_allocator_allocate_all_space() {
    u64 max_allocation_count = 1024;
    Linear_Allocator alloc;

    linear_allocator_create(
        sizeof(u64) * max_allocation_count,
        nullptr,
        &alloc);

    void* block;

    // In this test we will allocate a memory of 8 bytes 1024 times, to fill
    // completelly the linear allocator at the end of the loop and take all the
    // available space of the allocator
    for (u64 i = 0; i < max_allocation_count; ++i) {
        block = linear_allocator_allocate(&alloc, sizeof(u64));

        // Validate the allocation
        expect_should_not_be(nullptr, block);
        expect_should_be(sizeof(u64) * (i + 1), alloc.allocated);
    }

    linear_allocator_destroy(&alloc);

    return true;
}

u8 linear_allocator_overallocate() {
    u64 max_allocation_count = 3;
    Linear_Allocator alloc;

    linear_allocator_create(
        sizeof(u64) * max_allocation_count,
        nullptr,
        &alloc);

    void* block;

    // In this test we will allocate a memory of 8 bytes 1024 times, to fill
    // completelly the linear allocator at the end of the loop and take all the
    // available space of the allocator
    for (u64 i = 0; i < max_allocation_count; ++i) {
        block = linear_allocator_allocate(&alloc, sizeof(u64));

        // Validate the allocation
        expect_should_not_be(nullptr, block);
        expect_should_be(sizeof(u64) * (i + 1), alloc.allocated);
    }
    // At the end of this loop the memory buffer should be completelly filled

    ENGINE_DEBUG("Note: The following error is intentionally caused by the test");
    block = linear_allocator_allocate(&alloc, sizeof(u64));
    expect_should_be(nullptr, block);
    expect_should_not_be(sizeof(u64) * (max_allocation_count + 1), alloc.allocated);

    linear_allocator_destroy(&alloc);

    return true;
}

u8 linear_allocator_allocate_all_free_all() {
    u64 max_allocation_count = 1024;
    Linear_Allocator alloc;

    linear_allocator_create(
        sizeof(u64) * max_allocation_count,
        nullptr,
        &alloc);

    void* block;

    // In this test we will allocate a memory of 8 bytes 1024 times, to fill
    // completelly the linear allocator at the end of the loop and take all the
    // available space of the allocator
    for (u64 i = 0; i < max_allocation_count; ++i) {
        block = linear_allocator_allocate(&alloc, sizeof(u64));

        // Validate the allocation
        expect_should_not_be(nullptr, block);
        expect_should_be(sizeof(u64) * (i + 1), alloc.allocated);
    }
    // At the end of this loop the memory buffer should be completelly filled

    linear_allocator_free_all(&alloc);
    expect_should_be(0, alloc.allocated);

    linear_allocator_destroy(&alloc);

    return true;
}

void linear_allocator_register_tests() {
    test_manager_register_test(
        linear_allocator_should_create_and_destroy,
        "Linear allocator should create and destroy");

    test_manager_register_test(
        linear_allocator_allocate_all_space_single,
        "Linear allocator should allocate all available space with single allocations");

    test_manager_register_test(
        linear_allocator_allocate_all_space,
        "Linear allocator should allocate all available space with multi alloc operatios");

    test_manager_register_test(
        linear_allocator_overallocate,
        "Linear allocator should not allocate more than the space available");

    test_manager_register_test(
        linear_allocator_allocate_all_free_all,
        "Linear allocator should allocate all space with multiple allocations and free all space back");
}
