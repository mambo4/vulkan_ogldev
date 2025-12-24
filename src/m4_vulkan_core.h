#pragma once
#define GLFW_INCLUDE_VULKAN
#include "m4_vulkan_utils.h"
#include "m4_vulkan_device.h"
#include "m4_vulkan_queue.h"
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
            int GetSwapchainImageCount() const { return (int)m_swapChainImages.size(); }
            VkDevice GetDevice() const { return m_device; }//?need const?
            VkImage GetSwapchainImage(uint32_t imageIndex) const { return m_swapChainImages[imageIndex]; }
            VulkanQueue* GetQueue() { return &m_queue; }
            void CreateCommandBuffers(uint32_t commandBufferCount , VkCommandBuffer* pCommandBuffers);
            void FreeCommandBuffers(uint32_t commandBufferCount , VkCommandBuffer* pCommandBuffers);
            VkRenderPass CreateRenderPassSimple();
            std::vector<VkFramebuffer>CreateFrameBuffers(VkRenderPass renderPass);

        private:
            void CreateInstance(const char* pAppName);
            void CreateDebugCallback();
            void CreateSurface();
            void CreateDevice();
            void CreateSwapChain();
            void CreateCommandBufferPool();

            VkInstance m_instance = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
            GLFWwindow* m_pWindow = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
            VulkanPhysicalDevices m_physicalDevices;    
            uint32_t m_queueFamilyIndex = 0;
            VkDevice m_device;
            VkSurfaceFormatKHR m_swapChainSurfaceFormat;
            VkSwapchainKHR m_swapChain   = VK_NULL_HANDLE;
            std::vector<VkImage> m_swapChainImages; 
            std::vector<VkImageView> m_swapChainImageViews;
            std::vector<VkFramebuffer>m_frameBuffers;
            VkCommandPool m_commandBufferPool = VK_NULL_HANDLE;
            VulkanQueue m_queue;
            
    };

}