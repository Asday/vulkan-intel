#ifndef _c30e5db43d764c6a9164c33056aba9e3
#define _c30e5db43d764c6a9164c33056aba9e3

#include <stdexcept>

class VulkanNotFound: public std::runtime_error {
    public:
    VulkanNotFound() noexcept;
};

class MissingVulkanFunction: public std::runtime_error {
    public:
    MissingVulkanFunction(std::string which) noexcept;
};

class GenericVulkanError: public std::runtime_error {
    public:
    GenericVulkanError(std::string what) noexcept;
};

class NoSuitablePhysicalDevice: public std::runtime_error {
    public:
    NoSuitablePhysicalDevice() noexcept;
};

class DeviceCreationFailed: public std::runtime_error {
    public:
    DeviceCreationFailed() noexcept;
};

#endif
