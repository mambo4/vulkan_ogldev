#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#define WINDOW_WIDTH 1920   
#define WINDOW_HEIGHT 1080


GLFWwindow* window=NULL;

void GLFW_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

//todo: mouse callback

int main(int argc, char* argv[])
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Non-resizable window

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial01", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetKeyCallback(window, GLFW_Key_Callback);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}