#include "m4_vulkan_core.h"

//std
#include <stdio.h>
#include <stdlib.h>

//libs
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#define WINDOW_WIDTH 1280      
#define WINDOW_HEIGHT 720

#define APP_NAME "Tutorial02"

GLFWwindow* window=NULL;

void GLFW_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

//todo: mouse callback

class VulkanApp
{
    public:
        VulkanApp() {}
        ~VulkanApp() {}

        void Init(const char* pAppName, GLFWwindow* pWindow) 
        {
            m_vkCore.Init(pAppName, pWindow);
            m_numSwapchainImages = m_vkCore.GetSwapchainImageCount();
            m_pQueue = m_vkCore.GetQueue();
            CreateCommandBuffers();
            RecordCommandBuffers();
        }
        
        void RenderScene()
        {
            uint32_t imageIndex = m_pQueue->AcquireNextImage();
            m_pQueue->SubmitCommandBufferAsync(m_commandBuffers[imageIndex]);
            m_pQueue->PresentImage(imageIndex); 
        }

    private:

        void CreateCommandBuffers()
        {
            m_commandBuffers.resize(m_numSwapchainImages);
            m_vkCore.CreateCommandBuffers(m_numSwapchainImages, m_commandBuffers.data());
        }

        void RecordCommandBuffers()
        {

            VkClearColorValue clearColor = {  0.05f, 0.05f, 0.1f, 1.0f  };

            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;


            for (uint32_t i = 0; i < m_commandBuffers.size(); i++)
            {
                VkImageMemoryBarrier presentToClearBarrier = {};//prev to current image
                presentToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                presentToClearBarrier.pNext = VK_NULL_HANDLE;
                presentToClearBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                presentToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                presentToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                presentToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                presentToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                presentToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                presentToClearBarrier.image = m_vkCore.GetSwapchainImage(i);
                presentToClearBarrier.subresourceRange = subresourceRange;

                VkImageMemoryBarrier clearToPresentBarrier = {};//current to next image
                clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;   
                clearToPresentBarrier.pNext = VK_NULL_HANDLE;
                clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                clearToPresentBarrier.image = m_vkCore.GetSwapchainImage(i);
                clearToPresentBarrier.subresourceRange = subresourceRange;

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pNext = VK_NULL_HANDLE;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

                VkResult result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);
                CHECK_VK_RESULT(result, "vkBeginCommandBuffer");

                vkCmdPipelineBarrier(
                    m_commandBuffers[i],
                    VK_PIPELINE_STAGE_TRANSFER_BIT,//VK_PIPELINE_TOP_OF_PIPE_BIT?
                    VK_PIPELINE_STAGE_TRANSFER_BIT, 
                    0, 
                    0, VK_NULL_HANDLE, 
                    0, VK_NULL_HANDLE, 
                    1, &presentToClearBarrier);

                vkCmdClearColorImage(m_commandBuffers[i], m_vkCore.GetSwapchainImage(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);

                vkCmdPipelineBarrier(
                    m_commandBuffers[i],
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
                    0, 
                    0, VK_NULL_HANDLE, 
                    0, VK_NULL_HANDLE, 
                    1, &clearToPresentBarrier);

                result = vkEndCommandBuffer(m_commandBuffers[i]);
                CHECK_VK_RESULT(result, "vkEndCommandBuffer");

            }
        }

        m4VK::VulkanCore m_vkCore;
        m4VK::VulkanQueue* m_pQueue = VK_NULL_HANDLE;
        int m_numSwapchainImages = 0;
        std::vector<VkCommandBuffer> m_commandBuffers;
        
};

int main(int argc, char* argv[])
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Non-resizable window

    VulkanApp App;
     


    GLFWwindow* pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, NULL, NULL);

    if (!pWindow)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetKeyCallback(pWindow, GLFW_Key_Callback);

    App.Init(APP_NAME,pWindow);

    while (!glfwWindowShouldClose(pWindow))
    {
        App.RenderScene();
        glfwPollEvents();
    }

    glfwDestroyWindow(pWindow);
    glfwTerminate();
    return EXIT_SUCCESS;
}