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