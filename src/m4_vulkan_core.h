#pragma once
#define GLFW_INCLUDE_VULKAN
#include "m4_vulkan_utils.h"
#include "m4_vulkan_device.h"
#include <vulkan/vulkan.h>
#include <glfw3.h>

namespace m4VK
{
    class VulkanCore
    {
        public:
            VulkanCore();
            ~VulkanCore();

            void Init(const char* pAppName, GLFWwindow* pWindow);


        private:
            void CreateInstance(const char* pAppName);
            void CreateDebugCallback();
            void CreateSurface();
            void CreateDevice();
            void CreateSwapChain();

            VkInstance m_instance = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
            GLFWwindow* m_pWindow = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
            VulkanPhysicalDevices m_physicalDevices;
            uint32_t m_queueFamilyIndex = 0;
            VkDevice m_device;
            VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
            std::vector<VkImage> m_swapChainImages; 
            std::vector<VkImageView> m_swapChainImageViews;

    };

}