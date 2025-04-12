#pragma once

#include "defines.hpp"
#include "vulkan_types.hpp"

b8 vulkan_device_select(Vulkan_Context* context);

void vulkan_device_shutdown(Vulkan_Context* context);
