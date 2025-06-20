#include "core/logger.hpp"
#include "core/memory.hpp"
#include "memory/linear_allocator_tests.hpp"
#include "test_manager.hpp"
#include "platform/platform.hpp"

int main() {
    test_manager_init();

    // Test registration portion
    linear_allocator_register_tests();

    ENGINE_DEBUG("Starting tests...");

    test_manager_run_tests();

    return 0;
}
