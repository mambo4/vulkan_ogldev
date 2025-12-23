# pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <stdio.h>

namespace m4VK
{
    class VulkanQueue{

        public:
            static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

            VulkanQueue(){}
            ~VulkanQueue() {}

            void Init(VkDevice device,VkSwapchainKHR swapchain, uint32_t queueFamilyIndex, uint32_t queueIndex);
            void Destroy();
            uint32_t AcquireNextImage();
            void SubmitCommandBufferAsync(VkCommandBuffer commandBuffer);
            void SubmitCommandBufferSync(VkCommandBuffer commandBuffer);
            void PresentImage(uint32_t imageIndex);
            void WaitIdle();

        private: 
            
            void CreateSemaphores();
            VkDevice m_device= VK_NULL_HANDLE;
            VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
            VkQueue m_queue = VK_NULL_HANDLE;
            std::vector<VkSemaphore> m_imageAvailableWaitSemaphores;
            std::vector<VkSemaphore> m_renderFinishedSignalSemaphores;
            size_t m_currentFrame = 0;
    };
    
} // namespace m4VK
