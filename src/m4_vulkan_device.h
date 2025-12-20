#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace m4VK {

    struct PhysicalDevice {
        VkPhysicalDevice m_physicalDevice;
        VkPhysicalDeviceProperties m_deviceProperties;
        VkPhysicalDeviceFeatures m_deviceFeatures;
        std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
        // std::vector<VkExtensionProperties> m_ExtensionProperties;
        std::vector<VkBool32> m_queueFamilySupportsPresent;
        std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
        std::vector<VkPresentModeKHR> m_presentModes;
        VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
        VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties;
    };

    class VulkanPhysicalDevices {
    public:
        VulkanPhysicalDevices() {}
        ~VulkanPhysicalDevices() {}

        void Init(const VkInstance& instance, const VkSurfaceKHR& surface);
        uint32_t SelectPhysicalDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent);
        const PhysicalDevice& GetSelectedDevice() const;

    private:
        std::vector<PhysicalDevice> m_physicalDevices;
        int m_selectedDeviceIndex=-1;
    };

}