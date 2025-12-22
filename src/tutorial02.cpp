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
            CreateCommandBuffers();
        }
        void RenderScene()
        {
        }
    private:

        void CreateCommandBuffers()
        {
            m_commandBuffers.resize(m_numSwapchainImages);
            m_vkCore.CreateCommandBuffers(m_numSwapchainImages, m_commandBuffers.data());
        }

        void RecordCommandBuffers()
        {

            VkClearColorValue clearColor = {  0.0f, 0.0f, 0.0f, 1.0f  };

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
                beginInfo.flags = 0; //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

                VkResult result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);
                CHECK_VK_RESULT(result, "vkBeginCommandBuffer");

                vkCmdClearColorImage(m_commandBuffers[i], m_vkCore.GetSwapchainImage(i), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresourceRange);

                result = vkEndCommandBuffer(m_commandBuffers[i]);
                CHECK_VK_RESULT(result, "vkEndCommandBuffer");

            }
        }

        m4VK::VulkanCore m_vkCore;
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