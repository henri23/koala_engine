#include <vulkan/vulkan.h>

#include <iostream>

int main() {
    // Check if Vulkan is available
    uint32_t instanceExtensionCount = 0;

    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to enumerate Vulkan instance extensions!" << std::endl;
        return -1;
    }

    std::cout << "Vulkan is supported. Number of instance extensions: " << instanceExtensionCount << std::endl;

    // Create Vulkan instance
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Test App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkInstance instance;
    result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result == VK_SUCCESS) {
        std::cout << "Vulkan instance created successfully." << std::endl;

        // Destroy Vulkan instance
        vkDestroyInstance(instance, nullptr);
    } else {
        std::cerr << "Failed to create Vulkan instance!" << std::endl;
        return -1;
    }

    return 0;
}
