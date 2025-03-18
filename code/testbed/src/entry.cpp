#include <core/logger.hpp>
#include <core/asserts.hpp>
#include <defines.hpp>

#include <entry.hpp>

b8 create_game() {
    GAME_INFO("Called create_game()");


    RUNTIME_ASSERT_MSG(sizeof(u32) == 4, "Wrong uint32 size");
    RUNTIME_ASSERT(sizeof(u32) == 4);
    COMP_TIME_ASSERT(sizeof(u32) == 4, "wrong size");

    return TRUE;
}
