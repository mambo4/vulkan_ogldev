#include "glfw3.h"
#include "m4_vulkan_glfw.h"
#include "m4_glm_camera.h"

namespace m4VK{

    void GLFW_KeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
    {
        GLFWCallbacks* pGLFWCallbacks =(GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);
        pGLFWCallbacks->Key(pWindow, key,scancode,action, mods);
    }

    void GLFW_MouseMoveCallback(GLFWwindow* pWindow, double  xpos, double ypos)
    {
        GLFWCallbacks* pGLFWCallbacks =(GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);
        pGLFWCallbacks->MouseMove(pWindow, xpos, ypos);

    }

    void GLFW_MouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods)
    {
        GLFWCallbacks* pGLFWCallbacks =(GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);
        pGLFWCallbacks->MouseButton(pWindow,button,action,mods);

    }

    GLFWwindow* glfw_vulkan_init(int width, int height, const char* pTitle){
        
        if(!glfwInit())
        {
            exit(EXIT_FAILURE);
        }

        if(!glfwVulkanSupported())
        {
            exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,0);

        GLFWwindow* pWindow = glfwCreateWindow(width,height,pTitle, NULL, NULL);

        if(!pWindow)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        return pWindow;
    }

    void glfw_vulkan_set_callbacks(GLFWwindow* pWindow, GLFWCallbacks* pCallbacks)
    {
        glfwSetWindowUserPointer(pWindow, pCallbacks);
        glfwSetKeyCallback(pWindow,GLFW_KeyCallback);
        glfwSetCursorPosCallback(pWindow,GLFW_MouseMoveCallback);
        glfwSetMouseButtonCallback(pWindow, GLFW_MouseButtonCallback);
    }

}