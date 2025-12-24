
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
        M4_LOG("\n***** %s(%s) ************************************************************", GetDebugSeverityString(messageSeverity)    , GetDebugType(messageTypes));
        M4_LOG("%s", pCallbackData->pMessage);

        for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
        {
            const VkDebugUtilsObjectNameInfoEXT& obj = pCallbackData->pObjects[i];
            M4_LOG("\tObjects[%d]: %s:0x%llx ", i,GetObjectTypeString(obj.objectType), (unsigned long long)obj.objectHandle);
        }
        M4_LOG("************************************************************\n");
        return VK_FALSE;// The calling function should not be aborted
    }

    VulkanCore::VulkanCore(){}

    VulkanCore::~VulkanCore()
    {
        M4_LOG("\n----------- destructor ~VulkanCore() --------------");

        for (int i=0;i<m_frameBuffers.size();i++){
            vkDestroyFramebuffer(m_device, m_frameBuffers[i], VK_NULL_HANDLE);
            M4_LOG("vkDestroyFramebuffer[%d]",i  );
        }

        vkDestroyCommandPool(m_device, m_commandBufferPool, NULL);
        M4_LOG("vkDestroyCommandPool");

        m_queue.Destroy();
        
        for (int i=0;i<m_swapChainImageViews.size();i++)
        {
            vkDestroyImageView(m_device, m_swapChainImageViews[i], VK_NULL_HANDLE);
            M4_LOG("vkDestroyImageView[%d]",i  );
        }

        vkDestroySwapchainKHR(m_device, m_swapChain, VK_NULL_HANDLE);
        M4_LOG("vkDestroySwapchainKHR");


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
        M4_LOG("\n===================================================\n");
        M4_LOG("-----------  VulkanCore::Init() --------------\n");
        m_pWindow = pWindow;
        CreateInstance(pAppName);
        CreateDebugCallback();
        CreateSurface();
        m_physicalDevices.Init(m_instance, m_surface);
        m_queueFamilyIndex =  m_physicalDevices.SelectPhysicalDevice(VK_QUEUE_GRAPHICS_BIT, true);
        CreateDevice();
        CreateSwapChain();
        CreateCommandBufferPool();
        m_queue.Init(m_device, m_swapChain, m_queueFamilyIndex, 0);
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
        messengerCreateInfo.messageSeverity =//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                //    VK_DEBUG_REPORT_INFORMATION_BIT_EXT|
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
        M4_LOG("-------- Vulkan Instance Created Successfully --------");
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

    static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availableModes)
    {
        for (const auto& availableMode : availableModes)
        {
            if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)// mailbox mode made things waaay to fast in a prev tutorial
            {
                return availableMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    static uint32_t ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        uint32_t requestedNumImages = capabilities.minImageCount + 1;
        int finalNumImages=0;
        if( (capabilities.maxImageCount>0) && ( requestedNumImages > capabilities.maxImageCount +1) ){
            finalNumImages = capabilities.maxImageCount;
        }
        else{
            finalNumImages = requestedNumImages;
        }
        return finalNumImages;
    }

    static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }


    VkImageView CreateImageView( 
        VkDevice device, 
        VkImage image, 
        VkFormat format, 
        VkImageAspectFlags imageViewAspectFlags, 
        VkImageViewType viewType,
        uint32_t layerCount,
        uint32_t mipLevels)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = VK_NULL_HANDLE;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = viewType;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = imageViewAspectFlags;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = layerCount;

        VkImageView imageView;
        VkResult result = vkCreateImageView(device, &imageViewCreateInfo, VK_NULL_HANDLE, &imageView);
        CHECK_VK_RESULT(result, "Create image view");
        return imageView;
    }

    void VulkanCore::CreateSwapChain()
    {
        const VkSurfaceCapabilitiesKHR& surfaceCapabilities = m_physicalDevices.GetSelectedDevice().m_surfaceCapabilities;
        uint32_t numSurfaceImages = ChooseImageCount(surfaceCapabilities);

        const std::vector<VkPresentModeKHR>& presentModes = m_physicalDevices.GetSelectedDevice().m_presentModes;
        VkPresentModeKHR presentMode = ChoosePresentMode(presentModes);

        m_swapChainSurfaceFormat = ChooseSurfaceFormat(m_physicalDevices.GetSelectedDevice().m_surfaceFormats);
        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.pNext = VK_NULL_HANDLE;
        swapChainCreateInfo.flags = 0;
        swapChainCreateInfo.surface = m_surface;
        swapChainCreateInfo.minImageCount = numSurfaceImages;
        swapChainCreateInfo.imageColorSpace = m_swapChainSurfaceFormat.colorSpace;
        swapChainCreateInfo.imageFormat = m_swapChainSurfaceFormat.format;
        swapChainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 1;
        swapChainCreateInfo.pQueueFamilyIndices = &m_queueFamilyIndex;
        swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(m_device, &swapChainCreateInfo, VK_NULL_HANDLE, &m_swapChain);
        CHECK_VK_RESULT(result, "Create swap chain");
        M4_LOG("Vulkan Swap Chain Created Successfully");

        uint32_t swapchainImageCount = 0;
        result=vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapchainImageCount, nullptr);
        CHECK_VK_RESULT(result, "Get swap chain images count");
        assert(numSurfaceImages == swapchainImageCount);
        M4_LOG("swapchainImageCount:%d",swapchainImageCount);

        m_swapChainImages.resize(swapchainImageCount);
        m_swapChainImageViews.resize(swapchainImageCount);

        result=vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapchainImageCount, m_swapChainImages.data());
        CHECK_VK_RESULT(result, "Get swap chain images");

        int layerCount=1;
        int mipLevels=1;

        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            m_swapChainImageViews[i] = CreateImageView(
                m_device, 
                m_swapChainImages[i], 
                m_swapChainSurfaceFormat.format,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_2D,
                layerCount,
                mipLevels);
            M4_LOG("CreateImageView[%d]",i);
        }
    }

    void VulkanCore::CreateCommandBufferPool()
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.pNext = VK_NULL_HANDLE;
        poolInfo.flags = 0;//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_queueFamilyIndex;

        VkResult result = vkCreateCommandPool(m_device, &poolInfo, VK_NULL_HANDLE, &m_commandBufferPool);
        CHECK_VK_RESULT(result, "vkCreateCommandPool");
        M4_LOG("Vulkan Command Pool Created Successfully");
    }

    void VulkanCore::CreateCommandBuffers(uint32_t commandBufferCount , VkCommandBuffer* pCommandBuffers)
    {

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = VK_NULL_HANDLE;
        allocInfo.commandPool = m_commandBufferPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = commandBufferCount;

        VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, pCommandBuffers);
        CHECK_VK_RESULT(result, "vkAllocateCommandBuffers");
        M4_LOG("Vulkan Command Buffers Created Successfully");
    }

    void VulkanCore::FreeCommandBuffers(uint32_t commandBufferCount , VkCommandBuffer* pCommandBuffers)
    {
        vkFreeCommandBuffers(m_device, m_commandBufferPool, commandBufferCount, pCommandBuffers);
    }  

    VkRenderPass VulkanCore::CreateRenderPassSimple()
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_swapChainSurfaceFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {}; //make sure everything else init'd null or 0
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pNext = VK_NULL_HANDLE;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;

        VkRenderPass renderPass;
        VkResult result = vkCreateRenderPass(m_device, &renderPassInfo, VK_NULL_HANDLE, &renderPass);
        CHECK_VK_RESULT(result, "vkCreateRenderPass");
        M4_LOG("Vulkan Render Pass Created Successfully");

        return renderPass;
    }   

    std::vector<VkFramebuffer>VulkanCore::CreateFrameBuffers(VkRenderPass renderPass){

        m_frameBuffers.resize(m_swapChainImages.size());

        int width = m_physicalDevices.GetSelectedDevice().m_surfaceCapabilities.currentExtent.width;
        int height = m_physicalDevices.GetSelectedDevice().m_surfaceCapabilities.currentExtent.height;

        VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = VK_NULL_HANDLE;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = VK_NULL_HANDLE;
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1; 

        for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
        {
            framebufferInfo.pAttachments = &m_swapChainImageViews[i] ;

            VkResult result = vkCreateFramebuffer(m_device, &framebufferInfo, VK_NULL_HANDLE, &m_frameBuffers[i]);
            CHECK_VK_RESULT(result, "vkCreateFramebuffer");
            M4_LOG("Vulkan Framebuffer Created Successfully");
        }
        return m_frameBuffers;
    }
}