
# include "m4_vulkan_core.h"
# include "m4_vulkan_utils.h"
# include "m4_vulkan_device.h"
//std
# include <stdio.h>
# include <vector>
# include <assert.h>


// #define __apple__
// #define __linux__

namespace m4VK
{
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void*                                            pUserData)
    {
        M4_LOG("Debug Callback: %s", pCallbackData->pMessage);
        M4_LOG("\tSeverity:%s", GetDebugSeverityString(messageSeverity));
        M4_LOG("\tType:%s", GetDebugType(messageTypes));

        for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
        {
            const VkDebugUtilsObjectNameInfoEXT& obj = pCallbackData->pObjects[i];
            M4_LOG("\tObjects[%d]: %s:0x%llx ", i,GetObjectTypeString(obj.objectType), (unsigned long long)obj.objectHandle);
        }

        return VK_FALSE;// The calling function should not be aborted
    }

    VulkanCore::VulkanCore(){}

    VulkanCore::~VulkanCore()
    {
        M4_LOG("\n----------- destructor ~VulkanCore() --------------");

        vkDestroyDevice(m_device, VK_NULL_HANDLE);
        M4_LOG("vkDestroyDevice");

        if (m_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(m_instance, m_surface, VK_NULL_HANDLE);
            M4_LOG("vkDestroySurfaceKHR");
        }

        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
        vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (!vkDestroyDebugUtilsMessenger) {
            M4_ERROR("Cannot find address of vkDestroyDebugUtilsMessenger");
            exit(1);
        }

        if (m_debugMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, VK_NULL_HANDLE);
            M4_LOG("vkDestroyDebugUtilsMessenger");
        }
        vkDestroyInstance(m_instance, VK_NULL_HANDLE);
        M4_LOG("vkDestroyInstance");
    }

    void VulkanCore::Init(const char* pAppName, GLFWwindow* pWindow)
    {
        m_pWindow = pWindow;
        CreateInstance(pAppName);
        CreateDebugCallback();
        CreateSurface();
        m_physicalDevices.Init(m_instance, m_surface);
        m_queueFamilyIndex =  m_physicalDevices.SelectPhysicalDevice(VK_QUEUE_GRAPHICS_BIT, true);
        CreateDevice();
    }

    void VulkanCore::CreateInstance(const char *pAppName)
    {
        std::vector<const char*> Layers ={"VK_LAYER_KHRONOS_validation"};
 
	std::vector<const char*> Extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
		"VK_KHR_win32_surface",
#endif
#if defined (__APPLE__)
		"VK_MVK_macos_surface",
#endif
#if defined (__linux__)
		"VK_KHR_xcb_surface",
#endif
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	}; 
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
        messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.pNext = VK_NULL_HANDLE; 
        messengerCreateInfo.flags = 0;
        messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_REPORT_INFORMATION_BIT_EXT|
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; 
        messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerCreateInfo.pfnUserCallback = &DebugCallback;    
        messengerCreateInfo.pUserData = VK_NULL_HANDLE;

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = pAppName;
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "M4 Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = &messengerCreateInfo;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(Layers.size());
        instanceCreateInfo.ppEnabledLayerNames = Layers.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = Extensions.data();

        VkResult result = vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &m_instance);
        CHECK_VK_RESULT(result, "Create instance");
        M4_LOG("Vulkan Instance Created Successfull");
    }

    void VulkanCore::CreateDebugCallback()
    {
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
        messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.pNext = VK_NULL_HANDLE; 
        messengerCreateInfo.flags = 0;
        messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_REPORT_INFORMATION_BIT_EXT|
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; 
        messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerCreateInfo.pfnUserCallback = &DebugCallback;    
        messengerCreateInfo.pUserData = VK_NULL_HANDLE;

        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
        vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        if (!vkCreateDebugUtilsMessenger) {
            M4_ERROR("Cannot find address of vkCreateDebugUtilsMessenger");
            exit(1);
        }

        VkResult result = vkCreateDebugUtilsMessenger(m_instance, &messengerCreateInfo, VK_NULL_HANDLE, &m_debugMessenger);
        CHECK_VK_RESULT(result, "Create Debug Utils Messenger");
        M4_LOG("Debug Utils Messenger Created Successfully");
    }

    void VulkanCore::CreateSurface()
    {
        VkResult result = glfwCreateWindowSurface(m_instance, m_pWindow, VK_NULL_HANDLE, &m_surface);
        CHECK_VK_RESULT(result, "Create surface");
        M4_LOG("Vulkan Surface Created Successfully");
    }



    void VulkanCore::CreateDevice()
    {

        std::vector<const char*> deviceExtensions={
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
        };

        VkPhysicalDeviceFeatures DeviceFeatures = {0};
        DeviceFeatures.geometryShader = VK_TRUE;
        DeviceFeatures.tessellationShader = VK_TRUE;
        // DeviceFeatures.multiDrawIndirect = VK_TRUE;

        if( m_physicalDevices.GetSelectedDevice().m_deviceFeatures.geometryShader == VK_FALSE ){
            M4_ERROR("Geometry Shader unsupported");
        }

        if(m_physicalDevices.GetSelectedDevice().m_deviceFeatures.tessellationShader == VK_FALSE){
            M4_ERROR("Tessellation Shader unsupported");
        }


        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = VK_NULL_HANDLE;
        queueCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        float queuePriorities[] = { 1.0f };
        queueCreateInfo.pQueuePriorities = &queuePriorities[0];

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = VK_NULL_HANDLE;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.enabledLayerCount = 0; // Layers are deprecated
        deviceCreateInfo.ppEnabledLayerNames = VK_NULL_HANDLE;// Layers are deprecated
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.pEnabledFeatures = &DeviceFeatures; // Use VkPhysicalDeviceFeatures2 for more advanced features

        VkResult result = vkCreateDevice(m_physicalDevices.GetSelectedDevice().m_physicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &m_device);
        CHECK_VK_RESULT(result, "Create device");
        M4_LOG("Vulkan  Device Created Successfully");
    }   
}