#pragma once

#include <vulkan/vulkan.h>
#include <glfw3.h>

/* UTILS **********************************************************************/

#define CHECK_VK_RESULT(value, message) if (value != VK_SUCCESS) {\
     fprintf(stderr, "Error %s[%d]: %s(%x)\n", __FILE__, __LINE__, message, value);\
     exit(1); }

namespace m4VK {
    const char* GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT Severity);

    const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type);     
}

/* CORE **********************************************************************/
namespace m4VK
{
    class VulkanCore
    {
        public:
            VulkanCore();
            ~VulkanCore();

            void Init(const char* pAppName, GLFWwindow* pWindow);


        private:
            void CreateInstance(const char* pAppName);
            VkInstance m_instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
            void CreateDebugCallback();
            void CreateSurface(GLFWwindow* pWindow);
            const char* GetDebugSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT severity);
            const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT type);
            const char* GetObjectTypeString(VkObjectType type);

    };

}