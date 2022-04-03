#ifndef _2c0d3555ad1d49e58fde5c86a66d72b2
#define _2c0d3555ad1d49e58fde5c86a66d72b2

#include <vulkan/vulkan.h>

#include <string>

class Vulkan {
    void* vulkan;
    VkInstance instance{VK_NULL_HANDLE};
    uint32_t queueFamilyIndex{0};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue queue{VK_NULL_HANDLE};

    bool supportsLargeImages(VkPhysicalDevice device);
    bool hasGraphicsQueue(VkPhysicalDevice device);
    bool isSuitable(VkPhysicalDevice device);

    void selectPhysicalDevice(VkPhysicalDevice& physicalDevice);

    void loadVulkan();
    void createInstance(std::string name);
    void createDevice();

    #define VK_NO_PROTOTYPES
    #define VK_EXPORTED_FUNCTION(fun) PFN_##fun fun;
    #define VK_GLOBAL_LEVEL_FUNCTION(fun) PFN_##fun fun;
    #define VK_INSTANCE_LEVEL_FUNCTION(fun) PFN_##fun fun;
    #define VK_DEVICE_LEVEL_FUNCTION(fun) PFN_##fun fun;

    #include "all_vk_functions.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_NO_PROTOTYPES

    public:
    Vulkan(std::string name);
    ~Vulkan();
};

#endif
