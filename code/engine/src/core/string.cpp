#include "core/string.hpp"
#include <string.h>

b8 string_check_equal(
    const char* str1,
    const char* str2) {
	return strcmp(str1, str2) == 0;
}
