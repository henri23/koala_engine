#include "vulkan_object_shader.hpp"
#include "renderer/vulkan/vulkan_types.hpp"

b8 vulkan_object_shader_create(
    Vulkan_Context* context,
    Vulkan_Object_Shader* out_shader) {

	char stage_type_strs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};

    return TRUE;
}

void vulkan_object_shader_destroy(
    Vulkan_Context* context,
    Vulkan_Object_Shader* shader) {
}

void vulkan_object_shader_use(
    Vulkan_Context* context,
    Vulkan_Object_Shader* shader) {
}
