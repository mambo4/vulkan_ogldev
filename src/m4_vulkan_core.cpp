
# include "m4_vulkan_core.h"
# include <stdio.h>
# include <vector>

namespace m4VK
{
    VulkanCore::VulkanCore(){}

    VulkanCore::~VulkanCore()
    {
        vkDestroyInstance(m_instance, nullptr);
        printf("Vulkan Instance Destroyed\n");
    }

    void VulkanCore::Init(const char* pAppName)
    {
        CreateInstance(pAppName);
    }

    void VulkanCore::CreateInstance(const char *pAppName)
    {
        std::vector<const char*> Layers ={"VK_LAYER_KHRONOS_validation"};
 
        std::vector<const char*> Extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            "VK_KHR_win32_surface",
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
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

}