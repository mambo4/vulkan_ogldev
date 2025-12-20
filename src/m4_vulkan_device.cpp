#include "m4_vulkan_device.h"
#include "m4_vulkan_core.h"
//std
#include <assert.h>

namespace m4VK {


    static void PrintImageUsageFlags(const VkImageUsageFlags& flags)
    {
        if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
            printf("Image usage transfer src is supported\n");
        }

        if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            printf("Image usage transfer dest is supported\n");
        }

        if (flags & VK_IMAGE_USAGE_SAMPLED_BIT) {
            printf("Image usage sampled is supported\n");
        }

        if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            printf("Image usage color attachment is supported\n");
        }

        if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            printf("Image usage depth stencil attachment is supported\n");
        }

        if (flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
            printf("Image usage transient attachment is supported\n");
        }

        if (flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
            printf("Image usage input attachment is supported\n");
        }
    }


    static void PrintMemoryProperty(VkMemoryPropertyFlags PropertyFlags)
    {
        if (PropertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            printf("DEVICE LOCAL ");
        }

        if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            printf("HOST VISIBLE ");
        }

        if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            printf("HOST COHERENT ");
        }

        if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
            printf("HOST CACHED ");
        }

        if (PropertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
            printf("LAZILY ALLOCATED ");
        }

        if (PropertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
            printf("PROTECTED ");
        }
    }

    // VulkanPhysicalDevices::VulkanPhysicalDevices() {}
    // ~VulkanPhysicalDevices() {}

    void VulkanPhysicalDevices::Init(const VkInstance& instance, const VkSurfaceKHR& surface){

        uint32_t NumDevices = 0;

        VkResult result = vkEnumeratePhysicalDevices(instance, &NumDevices, VK_NULL_HANDLE);
        CHECK_VK_RESULT(result, "Failed to vkEnumeratePhysicalDevices");
        printf("Number of physical devices: %d\n", NumDevices);

        m_physicalDevices.resize(NumDevices);

        std::vector<VkPhysicalDevice> PhysicalDevices(NumDevices);
        PhysicalDevices.resize(NumDevices);

        result = vkEnumeratePhysicalDevices(instance, &NumDevices, PhysicalDevices.data());
        CHECK_VK_RESULT(result, "Failed to vkEnumeratePhysicalDevices");    
        for (uint32_t i = 0; i < NumDevices; ++i) {
            VkPhysicalDevice physicalDevice = PhysicalDevices[i];
            m_physicalDevices[i].m_physicalDevice = physicalDevice;

            vkGetPhysicalDeviceProperties(physicalDevice, &m_physicalDevices[i].m_deviceProperties);

            printf("Device Name: %s\n", m_physicalDevices[i].m_deviceProperties.deviceName);
            uint32_t apiVersion = m_physicalDevices[i].m_deviceProperties.apiVersion;
            printf("Device API Version: %d.%d.%d.%d\n",
                 VK_API_VERSION_VARIANT(apiVersion),
                 VK_VERSION_MAJOR(apiVersion), 
                 VK_VERSION_MINOR(apiVersion), 
                 VK_VERSION_PATCH(apiVersion));
            
            uint32_t NumQueueFamilies = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &NumQueueFamilies, VK_NULL_HANDLE);
            printf("Number of queue families: %d\n", NumQueueFamilies);

            m_physicalDevices[i].m_queueFamilyProperties.resize(NumQueueFamilies);
            m_physicalDevices[i].m_queueFamilySupportsPresent.resize(NumQueueFamilies);

            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &NumQueueFamilies, m_physicalDevices[i].m_queueFamilyProperties.data());

            for (uint32_t j = 0; j < NumQueueFamilies; j++) {
                const VkQueueFamilyProperties& QueueFamilyProperty = m_physicalDevices[i].m_queueFamilyProperties[j];
                printf("\tQueue Family [%d]: Queue Count: %d\n", j, QueueFamilyProperty.queueCount);
                VkQueueFlags Flags = QueueFamilyProperty.queueFlags;
                printf("\tGFX %s, Compute %s, Transfer %s, Sparse binding %s\n",
                    (Flags & VK_QUEUE_GRAPHICS_BIT) ? "True" : "False",
                    (Flags & VK_QUEUE_COMPUTE_BIT) ? "True" : "False",
                    (Flags & VK_QUEUE_TRANSFER_BIT) ? "True" : "False",
                    (Flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "True" : "False");
                result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, j, surface, &m_physicalDevices[i].m_queueFamilySupportsPresent[j]);
                CHECK_VK_RESULT(result, "Failed to vkGetPhysicalDeviceSurfaceSupportKHR\n");
            }

            uint32_t NumFormats = 0;
            result=vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &NumFormats, VK_NULL_HANDLE);
            CHECK_VK_RESULT(result, "Failed to vkGetPhysicalDeviceSurfaceFormatsKHR(1)\n");
            assert(NumFormats > 0);

            m_physicalDevices[i].m_surfaceFormats.resize(NumFormats);
            result=vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &NumFormats, m_physicalDevices[i].m_surfaceFormats.data());
            CHECK_VK_RESULT(result, "Failed to vkGetPhysicalDeviceSurfaceFormatsKHR(2)\n");

            for(uint32_t j=0; j<NumFormats; j++) {
                const VkSurfaceFormatKHR& Format=m_physicalDevices[i].m_surfaceFormats[j];
                printf("\tSurface format %d: Format: %d, Color Space: %d\n", j, Format.format, Format.colorSpace);
            }

            result=vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &m_physicalDevices[i].m_surfaceCapabilities);
            CHECK_VK_RESULT(result, "Failed to vkGetPhysicalDeviceSurfaceCapabilitiesKHR(1)\n");

            PrintImageUsageFlags(m_physicalDevices[i].m_surfaceCapabilities.supportedUsageFlags);

            uint32_t NumPresentModes = 0;
            result=vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &NumPresentModes, VK_NULL_HANDLE);
            CHECK_VK_RESULT(result, "Failed to vkGetPhysicalDeviceSurfacePresentModesKHR(1)\n");
            assert(NumPresentModes !=0);
            m_physicalDevices[i].m_presentModes.resize(NumPresentModes);
            result=vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &NumPresentModes, m_physicalDevices[i].m_presentModes.data());
            CHECK_VK_RESULT(result, "Failed to vkGetPhysicalDeviceSurfacePresentModesKHR(2)\n");

            printf("NumPresentModes: %d\n", NumPresentModes);

            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &(m_physicalDevices[i].m_deviceMemoryProperties));
            printf("Memory Types: %d\n", m_physicalDevices[i].m_deviceMemoryProperties.memoryTypeCount);
            for(uint32_t j=0; j<m_physicalDevices[i].m_deviceMemoryProperties.memoryTypeCount; j++) {
                printf("[%d]: flags %x heap %d ", j, 
                m_physicalDevices[i].m_deviceMemoryProperties.memoryTypes[j].propertyFlags,
                m_physicalDevices[i].m_deviceMemoryProperties.memoryTypes[j].heapIndex);

                PrintMemoryProperty(m_physicalDevices[i].m_deviceMemoryProperties.memoryTypes[j].propertyFlags);
                printf("\n");
            }
            printf("Memory Heaps: %d\n", m_physicalDevices[i].m_deviceMemoryProperties.memoryHeapCount);
            printf("\n");

            vkGetPhysicalDeviceFeatures(physicalDevice, &m_physicalDevices[i].m_deviceFeatures);
        }
        // exit(0);
    }

    uint32_t VulkanPhysicalDevices::SelectPhysicalDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent){
        for(uint32_t i=0; i<m_physicalDevices.size(); i++) {

            for(uint32_t j=0; j<m_physicalDevices[i].m_queueFamilyProperties.size(); j++) {

                const VkQueueFamilyProperties& QueueFamilyProperty=m_physicalDevices[i].m_queueFamilyProperties[j];

                if((QueueFamilyProperty.queueFlags & RequiredQueueType) && ((bool)m_physicalDevices[i].m_queueFamilySupportsPresent[j] == SupportsPresent)) {
                    m_selectedDeviceIndex=i;
                    int selectedQueueFamilyIndex=j;
                    printf("Using device [%d], queue family [%d]\n", i, j);
                    return selectedQueueFamilyIndex;
                }
            }
        }
        M4_ERROR("Required queue type %x and supports present %d not found\n", RequiredQueueType, SupportsPresent);
        return 0;
    }

    const PhysicalDevice& VulkanPhysicalDevices::GetSelectedDevice() const {
        if (m_selectedDeviceIndex <0){
            M4_ERROR("Device not selected\n");
        }
        return m_physicalDevices[m_selectedDeviceIndex];
    }
}