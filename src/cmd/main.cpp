#include "../Exceptions.h"
#include "../Vulkan.h"

#include <iostream>
#include <string>

using namespace std::literals::string_literals;

int main() {
    try {
        Vulkan("Unlimited Bepis Works"s);
    }
    catch (const VulkanNotFound&) {
        std::cout
            << "Vulkan doesn't appear to be installed on this machine."s
            << std::endl;

        return -1;
    }
    catch (const MissingVulkanFunction& e) {
        std::cout
            << e.what()
            << std::endl;

        return -2;
    }
    catch (const GenericVulkanError& e) {
        std::cout
            << e.what()
            << std::endl;

        return -3;
    }
    catch (const NoSuitablePhysicalDevice&) {
        std::cout
            << "No suitable GPUs appear to be installed in this machine."s
            << std::endl;

        return -4;
    }
    catch (const DeviceCreationFailed&) {
        // TODO: Improve user feedback.
        // Don't currently know why this would happen in real
        // circumstances.  When I find out, I should improve the user
        // feedback to guide them down the path I've yet to walk.
        std::cout
            << "Failed to create a logical device."s
            << std::endl;

        return -5;
    }

    return 0;
}
