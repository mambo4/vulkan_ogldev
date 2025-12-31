#pragma once

#include "m4_glm_camera.h"
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

namespace m4VK{

    class GLFWCallbacks{

        public:

        virtual void Key(GLFWwindow* pWindow, int key, int scancode, int action, int mods)=0;
        virtual void MouseMove(GLFWwindow* pWindow, double  xpos, double ypos)=0;
        virtual void MouseButton(GLFWwindow* pWindow, int button, int action, int mods)=0;

    };

    GLFWwindow* glfw_vulkan_init(int width, int height, const char* pTitle);

    void glfw_vulkan_set_callbacks(GLFWwindow* pWindow, GLFWCallbacks* pCallbacks);

}