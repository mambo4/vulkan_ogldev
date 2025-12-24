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
        ~VulkanApp() {
            m_vkCore.FreeCommandBuffers((uint32_t)m_commandBuffers.size(), m_commandBuffers.data());
            vkDestroyRenderPass(m_vkCore.GetDevice(), m_renderPass, VK_NULL_HANDLE);
        }

        void Init(const char* pAppName, GLFWwindow* pWindow) 
        {
            m_vkCore.Init(pAppName, pWindow);
            m_numSwapchainImages = m_vkCore.GetSwapchainImageCount();
            m_pQueue = m_vkCore.GetQueue();
            m_renderPass=m_vkCore.CreateRenderPassSimple();
            m_frameBuffers=m_vkCore.CreateFrameBuffers(m_renderPass);
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
            VkClearValue clearValue;
            clearValue.color = clearColor;

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = VK_NULL_HANDLE;
            renderPassBeginInfo.renderPass = m_renderPass;
            renderPassBeginInfo.renderArea.offset.x = 0;
            renderPassBeginInfo.renderArea.offset.y = 0;
            renderPassBeginInfo.renderArea.extent.width = WINDOW_WIDTH;
            renderPassBeginInfo.renderArea.extent.height = WINDOW_HEIGHT;
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = &clearValue;



            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;


            for (uint32_t i = 0; i < m_commandBuffers.size(); i++)
            {

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pNext = VK_NULL_HANDLE;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

                VkResult result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);
                CHECK_VK_RESULT(result, "vkBeginCommandBuffer");

                renderPassBeginInfo.framebuffer = m_frameBuffers[i];
                vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                
                // vkCmdClearColorImage(
                //     m_commandBuffers[i],
                //     m_vkCore.GetSwapchainImage(i), 
                //     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                //     &clearColor, 
                //     1, 
                //     &subresourceRange);  

                vkCmdEndRenderPass(m_commandBuffers[i]);

                result = vkEndCommandBuffer(m_commandBuffers[i]);
                CHECK_VK_RESULT(result, "vkEndCommandBuffer");

            }
        }

        m4VK::VulkanCore m_vkCore;
        m4VK::VulkanQueue* m_pQueue = VK_NULL_HANDLE;
        int m_numSwapchainImages = 0;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VkFramebuffer> m_frameBuffers;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
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