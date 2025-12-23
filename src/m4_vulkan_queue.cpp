# include <vulkan/vulkan.h>
# include "m4_vulkan_utils.h"
# include "m4_vulkan_queue.h"

namespace m4VK {

    void VulkanQueue::Init(VkDevice device, VkSwapchainKHR swapChain, uint32_t queue_family_index, uint32_t queue_index) {

        m_device = device;
        m_swapChain = swapChain;

        vkGetDeviceQueue(m_device, queue_family_index, queue_index, &m_queue);

        M4_LOG("VulkanQueue::Init: Queue initialized successfully.");

        CreateSemaphores();
    }

    void VulkanQueue::Destroy() {
        if (m_device != VK_NULL_HANDLE) {

            for (uint32_t i = 0; i < m_imageAvailableWaitSemaphores.size(); i++) {
                vkDestroySemaphore(m_device, m_imageAvailableWaitSemaphores[i], nullptr);
                vkDestroySemaphore(m_device, m_renderFinishedSignalSemaphores[i], nullptr);
            }

            M4_LOG("VulkanQueue::Destroy: Semaphores destroyed successfully.");
        }
    }

    void VulkanQueue::CreateSemaphores() {
        /*
        Same issue as with Brendan Galea https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR

        creating only a single  "wait" (m_imageAvailableSemaphore) semaphore 
        and a single "signal" (m_renderFinishedSemaphore) semaphore for all swapchain frames is not good enough
        we need to create a semaphore array/vector, and store for each frame in the swapchain
        
        comments https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Frames_in_flight

        */
        uint32_t swapchainImageCount = 0;
        vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapchainImageCount, nullptr);


        m_imageAvailableWaitSemaphores.resize(swapchainImageCount);
        m_renderFinishedSignalSemaphores.resize(swapchainImageCount);

         for (uint32_t i = 0; i < swapchainImageCount; i++) {
            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkResult result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableWaitSemaphores[i]);
            CHECK_VK_RESULT(result, "Failed to create image available semaphore");

            result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSignalSemaphores[i]);
            CHECK_VK_RESULT(result, "Failed to create render finished semaphore");
        }

    }

    void VulkanQueue::WaitIdle() {
        vkQueueWaitIdle(m_queue);
    }

    uint32_t VulkanQueue::AcquireNextImage() {
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableWaitSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
        CHECK_VK_RESULT(result, "Failed to acquire next image from swapchain");
        return imageIndex;
    }

    void VulkanQueue::SubmitCommandBufferAsync(VkCommandBuffer commandBuffer) {
        VkSubmitInfo submitInfo = {};

        VkSemaphore waitSemaphores[] = {m_imageAvailableWaitSemaphores[m_currentFrame]};
        VkSemaphore signalSemaphores[] = {m_renderFinishedSignalSemaphores[m_currentFrame]};
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult result = vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
        CHECK_VK_RESULT(result, "vkQueueSubmit Failed to submit command buffer");

    }

    // void VulkanQueue::SubmitCommandBufferSync(VkCommandBuffer commandBuffer) {
    //     VkSubmitInfo submitInfo = {};
    //     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //     submitInfo.waitSemaphoreCount = 0;
    //     submitInfo.pWaitSemaphores = VK_NULL_HANDLE;

    //     submitInfo.commandBufferCount = 1;
    //     submitInfo.pCommandBuffers = &commandBuffer;

    //     submitInfo.signalSemaphoreCount = 0;
    //     submitInfo.pWaitSemaphores = VK_NULL_HANDLE;

    //     VkResult result = vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    //     CHECK_VK_RESULT(result, "vkQueueSubmit Failed to submit command buffer");
    // }

    void VulkanQueue::PresentImage(uint32_t imageIndex) {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_renderFinishedSignalSemaphores[m_currentFrame];

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain;
        presentInfo.pImageIndices = &imageIndex; 

        VkResult result = vkQueuePresentKHR(m_queue, &presentInfo);
        CHECK_VK_RESULT(result, "vkQueuePresentKHR Failed to present image");
        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        WaitIdle();

    }
}