#include "Vulkan.h"

#include "Exceptions.h"

#include <dlfcn.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <iostream>

using namespace std::literals::string_literals;

bool Vulkan::supportsLargeImages(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    return (
        1 <= VK_VERSION_MAJOR(properties.apiVersion)
        || 4096 <= properties.limits.maxImageDimension2D
    );
}

bool Vulkan::hasGraphicsQueue(VkPhysicalDevice device) {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queueFamilyCount,
        nullptr
    );
    if (queueFamilyCount == 0) return false;

    std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queueFamilyCount,
        properties.data()
    );

    auto property = std::ranges::find_if(
        properties,
        [](VkQueueFamilyProperties p) {
            return p.queueCount > 0 && p.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        }
    );

    if (properties.end() != property) {
        queueFamilyIndex = std::distance(properties.begin(), property);

        return true;
    }

    return false;
}

bool Vulkan::isSuitable(VkPhysicalDevice device) {
    return supportsLargeImages(device) && hasGraphicsQueue(device);
}

void Vulkan::loadVulkan() {
    vulkan = dlopen("libvulkan.so", RTLD_NOW);

    if (vulkan == nullptr) throw VulkanNotFound();

    #define VK_EXPORTED_FUNCTION(fun) \
    if (!(fun = (PFN_##fun)dlsym(vulkan, #fun))) \
        throw MissingVulkanFunction(#fun);

    #include "exported_vk_functions.inl"

    #define VK_GLOBAL_LEVEL_FUNCTION(fun) \
    if (!(fun = (PFN_##fun)vkGetInstanceProcAddr(nullptr, #fun))) \
        throw MissingVulkanFunction(#fun);

    #include "global_level_vk_functions.inl"
}

void Vulkan::createInstance(std::string name) {
    uint32_t apiVersion;
    if (VK_SUCCESS != vkEnumerateInstanceVersion(&apiVersion))
        throw GenericVulkanError("failed to get Vulkan version"s);

    VkApplicationInfo applicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = name.c_str(),
        .applicationVersion = 0,
        .pEngineName = "vulkan-intel",
        .engineVersion = 0,
        .apiVersion = apiVersion,
    };
    VkInstanceCreateInfo createInstanceInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
    };

    // Possible errors:
    // `VK_ERROR_OUT_OF_HOST_MEMORY`
    // `VK_ERROR_OUT_OF_DEVICE_MEMORY`
    // `VK_ERROR_INITIALIZATION_FAILED`
    // `VK_ERROR_LAYER_NOT_PRESENT`
    // `VK_ERROR_EXTENSION_NOT_PRESENT`
    // `VK_ERROR_INCOMPATIBLE_DRIVER`
    if (VK_SUCCESS != vkCreateInstance(&createInstanceInfo, nullptr, &instance))
        throw GenericVulkanError("failed to create Vulkan instance"s);

    #define VK_INSTANCE_LEVEL_FUNCTION(fun) \
    if (!(fun = (PFN_##fun)vkGetInstanceProcAddr(instance, #fun))) \
        throw MissingVulkanFunction(#fun);

    #include "instance_level_vk_functions.inl"
}

void Vulkan::selectPhysicalDevice(VkPhysicalDevice& physicalDevice) {
    // Possible errors:
    // `VK_ERROR_OUT_OF_HOST_MEMORY`
    // `VK_ERROR_OUT_OF_DEVICE_MEMORY`
    // `VK_ERROR_INITIALIZATION_FAILED`
    // Can also return `VK_INCOMPLETE` on success when
    // `pPhysicalDeviceCount` is smaller than the amount of
    // `pPhysicalDevices` read.  Clearly not important here.
    uint32_t deviceCount;
    if (VK_SUCCESS != vkEnumeratePhysicalDevices(
        instance,
        &deviceCount,
        nullptr
    ))
        throw GenericVulkanError("failed to count physical devices"s);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (VK_SUCCESS != vkEnumeratePhysicalDevices(
        instance,
        &deviceCount,
        devices.data()
    ))
        throw GenericVulkanError("failed to enumerate physical devices"s);

    auto result = std::ranges::find_if(
        devices,
        [&](VkPhysicalDevice device){
            return isSuitable(device);
        }
    );
    if (devices.end() == result) throw NoSuitablePhysicalDevice();

    physicalDevice = *result;
}

void Vulkan::createDevice() {
    VkPhysicalDevice physicalDevice;
    selectPhysicalDevice(physicalDevice);

    std::vector<float> priorities{1.0f};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = static_cast<uint32_t>(priorities.size()),
            .pQueuePriorities = priorities.data(),
        },
    };
    VkDeviceCreateInfo deviceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures = nullptr,
    };
    if (VK_SUCCESS != vkCreateDevice(
        physicalDevice,
        &deviceCreateInfo,
        nullptr,
        &device
    ))
        throw DeviceCreationFailed();

    #define VK_DEVICE_LEVEL_FUNCTION(fun) \
    if (!(fun = (PFN_##fun)vkGetDeviceProcAddr(device, #fun))) \
        throw MissingVulkanFunction(#fun);

    #include "device_level_vk_functions.inl"

    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

Vulkan::Vulkan(std::string name) {
    loadVulkan();
    createInstance(name);
    createDevice();
}

Vulkan::~Vulkan() {
    if (VK_NULL_HANDLE != device) {
        vkDeviceWaitIdle(device);
        vkDestroyDevice(device, nullptr);
    }

    if (VK_NULL_HANDLE != instance) vkDestroyInstance(instance, nullptr);

    if (vulkan) dlclose(vulkan);
}
