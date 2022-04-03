#include "Exceptions.h"

#include <sstream>
#include <stdexcept>
#include <string>

using namespace std::literals::string_literals;

VulkanNotFound::VulkanNotFound() noexcept:
    std::runtime_error("Vulkan not found"s) {}

MissingVulkanFunction::MissingVulkanFunction(const std::string which) noexcept:
    std::runtime_error(
        (
            std::stringstream{}
            << "missing Vulkan function `"s
            << which
            << "`"s
        ).str()
    ) {}

GenericVulkanError::GenericVulkanError(const std::string what) noexcept:
    std::runtime_error(what) {}

NoSuitablePhysicalDevice::NoSuitablePhysicalDevice() noexcept:
    std::runtime_error("no suitable physical device found"s) {};

DeviceCreationFailed::DeviceCreationFailed() noexcept:
    std::runtime_error("failed to create device"s) {};
