#pragma once

#include "renderer/renderer_types.inl"
#include "renderer/vulkan/vulkan_types.hpp"

b8 vulkan_object_shader_create(
    Vulkan_Context* context,
    Vulkan_Object_Shader* out_shader);

void vulkan_object_shader_destroy(
    Vulkan_Context* context,
    Vulkan_Object_Shader* shader);

void vulkan_object_shader_use(
    Vulkan_Context* context,
    Vulkan_Object_Shader* shader);
