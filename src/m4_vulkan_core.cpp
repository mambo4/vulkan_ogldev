
# include "m4_vulkan_core.h"
# include <stdio.h>
# include <vector>

namespace m4VK
{
    VulkanCore::VulkanCore(){}

    VulkanCore::~VulkanCore()
    {

        if (m_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(m_instance, m_surface, VK_NULL_HANDLE);
            printf("Vulkan Surface Destroyed\n");
        }

        if (m_debugMessenger != VK_NULL_HANDLE)
        {
            PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
            vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
            if (vkDestroyDebugUtilsMessenger != VK_NULL_HANDLE) {
                vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, VK_NULL_HANDLE);
                printf("Debug Utils Messenger Destroyed\n");
            }
        }
        vkDestroyInstance(m_instance, VK_NULL_HANDLE);
        printf("Vulkan Instance Destroyed\n");
    }

    void VulkanCore::Init(const char* pAppName, GLFWwindow* pWindow)
    {
        CreateInstance(pAppName);
        CreateDebugCallback();
        CreateSurface(pWindow);
    }

    void VulkanCore::CreateInstance(const char *pAppName)
    {
        std::vector<const char*> Layers ={"VK_LAYER_KHRONOS_validation"};
 
        std::vector<const char*> Extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            "VK_KHR_win32_surface",
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };   

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = pAppName;
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "M4 Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(Layers.size());
        createInfo.ppEnabledLayerNames = Layers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
        createInfo.ppEnabledExtensionNames = Extensions.data();

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
        CHECK_VK_RESULT(result, "Create instance");
        printf("Vulkan Instance Created Successfully\n");
    }

    /* DEBUG UTILS **********************************************************************/
    const char* GetDebugSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT Severity)
    {
        switch (Severity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                return "VERBOSE";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                return "INFO";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                return "WARNING";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }
    
    const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type)
    {
        switch (Type)
        {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                return "GENERAL";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                return "VALIDATION";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                return "PERFORMANCE";
            default:
                return "UNKNOWN";
        }
    }     

    const char* GetObjectTypeString(VkObjectType type)
    {
        switch (type)
        {
            case VK_OBJECT_TYPE_INSTANCE:
                return "INSTANCE";
            case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
                return "PHYSICAL_DEVICE";
            case VK_OBJECT_TYPE_DEVICE:
                return "DEVICE";
            case VK_OBJECT_TYPE_QUEUE:
                return "QUEUE";
            case VK_OBJECT_TYPE_SEMAPHORE:
                return "SEMAPHORE";
            case VK_OBJECT_TYPE_COMMAND_BUFFER:
                return "COMMAND_BUFFER";
            // Add more cases as needed
            default:
                return "UNKNOWN_OBJECT_TYPE";
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void*                                            pUserData)
    {
        printf("Debug Callback: %s\n", pCallbackData->pMessage);
        printf("    Severity:%s\n", GetDebugSeverityString(messageSeverity));
        printf("    Type:%s\n", GetDebugType(messageTypes));
        printf("    Objects: ");

        for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
        {
            const VkDebugUtilsObjectNameInfoEXT& obj = pCallbackData->pObjects[i];
            printf(" [%s:0x%llx] ", GetObjectTypeString(obj.objectType), (unsigned long long)obj.objectHandle);
        }

        return VK_FALSE;
    }

    void VulkanCore::CreateDebugCallback()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.pNext = nullptr; 
        createInfo.flags = 0;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_REPORT_INFORMATION_BIT_EXT|
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; 
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = &DebugCallback;    
        createInfo.pUserData = nullptr;

        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = nullptr;
        vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        if (vkCreateDebugUtilsMessenger == nullptr) {
            fprintf(stderr, "Could not load vkCreateDebugUtilsMessengerEXT\n");
            exit(1);
        }   

        VkResult result = vkCreateDebugUtilsMessenger(m_instance, &createInfo, nullptr, &m_debugMessenger);
        CHECK_VK_RESULT(result, "Create Debug Utils Messenger");
        printf("Debug Utils Messenger Created Successfully\n");
    }


    void VulkanCore::CreateSurface(GLFWwindow* pWindow)
    {
        VkResult result = glfwCreateWindowSurface(m_instance, pWindow, nullptr, &m_surface);
        CHECK_VK_RESULT(result, "Create surface");
        printf("Vulkan Surface Created Successfully\n");
    }
}