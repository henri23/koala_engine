#pragma once

#include "defines.hpp"
#include "vulkan_types.hpp"

b8 vulkan_device_initialize(Vulkan_Context* context,
                        Vulkan_Physical_Device_Requirements* requirements);

void vulkan_device_shutdown(Vulkan_Context* context);
