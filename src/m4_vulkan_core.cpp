
# include "m4_vulkan_core.h"
# include "m4_vulkan_utils.h"
# include "m4_vulkan_device.h"

//libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//std
# include <stdio.h>
# include <vector>
# include <assert.h>

#define TEXTURE_FORMAT VK_FORMAT_R8G8B8A8_SRGB

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

        vkFreeCommandBuffers(m_device, m_commandBufferPool, 1, &m_copyCommandBuffer);
        vkDestroyCommandPool(m_device, m_commandBufferPool, NULL);

        M4_LOG("vkDestroyCommandPool");

        m_queue.Destroy();
        
        for (int i=0;i<m_swapChainImageViews.size();i++)
        {
            vkDestroyImageView(m_device, m_swapChainImageViews[i], VK_NULL_HANDLE);
            M4_LOG("vkDestroyImageView[%d]",i  );
        }
        
        //if (m_depthEnabled) {
		//  for (int i = 0; i < m_depthImages.size(); i++) {
		// 	    m_depthImages[i].Destroy(m_device);
		//  }
        //}

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
        CreateCommandBuffers(1,&m_copyCommandBuffer);
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

    void VulkanCore::CreateTexture(const char* pFilename, VulkanTexture& texture)
    {
        int imageWidth=0;
        int imageHeight=0;
        int imageChannels=0;

        //1 load pixels
        stbi_uc* pPixels=stbi_load(pFilename, &imageWidth,&imageHeight, &imageChannels, STBI_rgb_alpha);

        if(!pPixels){
            M4_ERROR("loading texture %s", pFilename);
            exit(1);
        }
        VkFormat format=TEXTURE_FORMAT;
        CreateTextureFromData(texture,pPixels,imageWidth,imageHeight,format,false);

        //3 free pixels
        stbi_image_free(pPixels);

        M4_LOG("Create texture (%s)",pFilename);
    }

    void VulkanCore::CreateImage(
        VulkanTexture& texture, 
        uint32_t imageWidth,
        uint32_t imageHeight,
        VkFormat format,
        VkImageUsageFlags usage, 
        VkMemoryPropertyFlagBits propertyFlags, 
        bool isCubemap)
    {
        VkExtent3D extent{};
        extent.width=imageWidth;
        extent.height=imageHeight;
        extent.depth=1;

        VkImageCreateInfo infoImage{};
        infoImage.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        infoImage.pNext=VK_NULL_HANDLE;
        infoImage.flags=isCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : (VkImageCreateFlags)0;
        infoImage.usage=usage;
        infoImage.format=format;
        infoImage.imageType=VK_IMAGE_TYPE_2D;
        infoImage.extent=extent;
        infoImage.mipLevels=1;
        infoImage.arrayLayers=isCubemap ? 6u : 1u;
        infoImage.samples=VK_SAMPLE_COUNT_1_BIT;
        infoImage.tiling=VK_IMAGE_TILING_OPTIMAL;
        infoImage.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
        infoImage.queueFamilyIndexCount=0;
        infoImage.pQueueFamilyIndices=VK_NULL_HANDLE;
        infoImage.initialLayout= VK_IMAGE_LAYOUT_UNDEFINED;

        VkResult result=vkCreateImage(m_device, &infoImage,VK_NULL_HANDLE, &texture.m_image);
        CHECK_VK_RESULT(result, "vkCreateImage");

        //2.0.1 get buffer requirements
        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device, texture.m_image, &memoryRequirements);
        M4_LOG("image needs %d bytes",(int)memoryRequirements.size);

        //2.0.2 get memoryTypeIndex
        uint32_t memoryTypeIndex= GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, propertyFlags);
        M4_LOG("memory type index %d", memoryTypeIndex);

        // 2.0.3 allocate
        VkMemoryAllocateInfo infoMemoryAllocate{};
        infoMemoryAllocate.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        infoMemoryAllocate.allocationSize=memoryRequirements.size;
        infoMemoryAllocate.memoryTypeIndex=memoryTypeIndex;

        result=vkAllocateMemory(m_device,&infoMemoryAllocate,VK_NULL_HANDLE,&texture.m_memory);
        CHECK_VK_RESULT(result, "vkAllocateMemory");

        //2.0.4 bind
        result=vkBindImageMemory(m_device,texture.m_image,texture.m_memory,0);
        CHECK_VK_RESULT(result, "vkBindImageMemory");
    }

    void VulkanCore::CreateImageFromData(
        VulkanTexture& texture, 
        const void* pPixels,
        uint32_t imageWidth,
        uint32_t imageHeight,
        VkFormat format, 
        bool isCubemap)
    {
        VkImageUsageFlagBits usage=(VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
        VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        CreateImage(texture,imageWidth,imageHeight,format,usage,propertyFlags,isCubemap);
        int layerCount= isCubemap?6:1;
        UpdateTextureImage(texture,imageWidth,imageHeight,format,pPixels,isCubemap);
    }
    
    void VulkanCore::UpdateTextureImage(
        VulkanTexture& texture,
        uint32_t imageWidth,
        uint32_t imageHeight,
        VkFormat format,
        const void* pPixels,
        bool isCubemap)
    {
        int bytesPerPixel=m4VK::GetBytesPerPixel(format); // always 4 for now
        VkDeviceSize layerSize= imageWidth*imageHeight*bytesPerPixel;
        int layerCount= isCubemap?6:1;
        VkDeviceSize imageSize=layerCount*layerSize;
        VkBufferUsageFlags bufferUsage=VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        BufferAndMemory stagingTexture = CreateBuffer(imageSize, bufferUsage, properties);
        stagingTexture.Update(m_device,pPixels, imageSize);
        stagingTexture.Log("UpdateTextureImage: stagingTexture");
        TransitionImageLayout(texture.m_image,format,VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(texture.m_image,stagingTexture.m_buffer,imageWidth,imageHeight);
        TransitionImageLayout(texture.m_image,format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        stagingTexture.Destroy(m_device); 
    }

    void VulkanCore::CreateTextureFromData(
        VulkanTexture& texture,
        const void* pPixels, 
        uint32_t imageWidth,
        uint32_t imageHeight,
        VkFormat format,
        bool isCubemap)
    {
        CreateImageFromData(texture,pPixels,imageWidth, imageHeight, format, isCubemap);
        VkImageAspectFlagBits aspect= VK_IMAGE_ASPECT_COLOR_BIT;
        texture.m_imageView=CreateImageView(m_device,texture.m_image,format,aspect,isCubemap);

        VkFilter minFilter=VK_FILTER_LINEAR;
        VkFilter maxFilter=VK_FILTER_LINEAR;
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        texture.m_sampler=CreateTextureSampler(m_device,minFilter,maxFilter,addressMode);

        M4_LOG("CreateTextureFromData.");
    }

    VkSampler VulkanCore::CreateTextureSampler(
        VkDevice device, 
        VkFilter minFilter, 
        VkFilter maxFilter, 
        VkSamplerAddressMode addressMode)
        {
            VkSamplerCreateInfo infoSampler{};
            infoSampler.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            infoSampler.magFilter=minFilter; // seems odd
            infoSampler.minFilter=maxFilter; // seems odd
            infoSampler.addressModeU=addressMode;
            infoSampler.addressModeV=addressMode;
            infoSampler.addressModeW=addressMode;
            infoSampler.mipLodBias=0.0f;
            infoSampler.anisotropyEnable=VK_FALSE;
            infoSampler.compareEnable=VK_FALSE;
            infoSampler.compareOp=VK_COMPARE_OP_ALWAYS;
            infoSampler.minLod=0.0f;
            infoSampler.maxLod=0.0f;
            infoSampler.borderColor=VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            infoSampler.unnormalizedCoordinates=VK_FALSE;

            VkSampler sampler;
            VkResult result=vkCreateSampler(device,&infoSampler,VK_NULL_HANDLE,&sampler);
            CHECK_VK_RESULT(result,"vkCreateSampler")
            return sampler;
        }

    void VulkanCore::TransitionImageLayout(
        VkImage& image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout)
    {
        //BeginCommandBuffer
        VkCommandBufferBeginInfo infoCommandBufferBegin = {};
        infoCommandBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        infoCommandBufferBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkResult result = vkBeginCommandBuffer(m_copyCommandBuffer, &infoCommandBufferBegin);
        CHECK_VK_RESULT(result, "vkBeginCommandBuffer");

        ImageMemBarrier(m_copyCommandBuffer, image,format, oldLayout, newLayout);

        //SubmitCopyCommand()
        vkEndCommandBuffer(m_copyCommandBuffer);
        m_queue.SubmitCommandBufferSync(m_copyCommandBuffer);
        m_queue.WaitIdle();
    }

    void VulkanCore::ImageMemBarrier(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout=oldLayout;
        barrier.newLayout=newLayout;
        barrier.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        barrier.image=image;

        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.levelCount=1;
        subresourceRange.layerCount=1;

        barrier.subresourceRange=subresourceRange;

        VkPipelineStageFlags sourceStage=VK_PIPELINE_STAGE_NONE;
        VkPipelineStageFlags destinationStage=VK_PIPELINE_STAGE_NONE;

        
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            (format == VK_FORMAT_D16_UNORM) ||
            (format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
            (format == VK_FORMAT_D32_SFLOAT) ||
            (format == VK_FORMAT_S8_UINT) ||
            (format == VK_FORMAT_D16_UNORM_S8_UINT) ||
            (format == VK_FORMAT_D24_UNORM_S8_UINT))
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            // if HasStencilComponent
            if ((format == VK_FORMAT_D32_SFLOAT_S8_UINT) || 
		        (format == VK_FORMAT_D24_UNORM_S8_UINT)) 
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } /* Convert back from read-only to updateable */
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } /* Convert from updateable texture to shader read-only */
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
                newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert depth texture from undefined state to depth-stencil buffer */
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } /* Wait for render pass to complete */
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = 0;
            /*
                    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            ///		destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                    destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            */
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert back from read-only to color attachment */
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } /* Convert from updateable texture to shader read-only */
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert back from read-only to depth attachment */
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        } /* Convert from updateable depth texture to shader read-only */
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } 
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = 0;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        else {
            printf("Unknown barrier case\n");
            exit(1);
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);

    }

    void  VulkanCore::CopyBufferToImage(VkImage destination, VkBuffer source, uint32_t imageWidth,uint32_t imageHeight){
        
        //BeginCommandBuffer
        VkCommandBufferBeginInfo infoCommandBufferBegin = {};
        infoCommandBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        infoCommandBufferBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkResult result = vkBeginCommandBuffer(m_copyCommandBuffer, &infoCommandBufferBegin);
        CHECK_VK_RESULT(result, "vkBeginCommandBuffer");

        
        //  check values for .imageOffset, see if its VkOffset3D w/ init xyz to zero
        VkBufferImageCopy bufferImageCopy{};
        
        VkImageSubresourceLayers imageSubresource{};
        imageSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT,
        imageSubresource.layerCount=1;

        bufferImageCopy.imageSubresource=imageSubresource;

        VkOffset3D offset{}; //should init xyz to zero
        bufferImageCopy.imageOffset=offset;

        VkExtent3D extent{};
        extent.width=imageWidth;
        extent.height=imageHeight;
        extent.depth=1;
        bufferImageCopy.imageExtent=extent;

        vkCmdCopyBufferToImage(
            m_copyCommandBuffer,
            source,
            destination,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &bufferImageCopy);

        //SubmitCopyCommand()
        vkEndCommandBuffer(m_copyCommandBuffer);
        m_queue.SubmitCommandBufferSync(m_copyCommandBuffer);
        m_queue.WaitIdle();

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


    VkImageView VulkanCore::CreateImageView( 
        VkDevice device, 
        VkImage image, 
        VkFormat format, 
        VkImageAspectFlags imageViewAspectFlags, 
        bool isCubemap)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = VK_NULL_HANDLE;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = imageViewAspectFlags;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = isCubemap ? 6u : 1u;

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

        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            m_swapChainImageViews[i] = CreateImageView(
                m_device, 
                m_swapChainImages[i], 
                m_swapChainSurfaceFormat.format,
                VK_IMAGE_ASPECT_COLOR_BIT,
                false);
            M4_LOG("CreateImageView[%d]",i);
        }
    }

    void VulkanCore::CreateCommandBufferPool()
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.pNext = VK_NULL_HANDLE;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
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

    std::vector<VkFramebuffer>VulkanCore::CreateFrameBuffer(VkRenderPass renderPass){

        std::vector<VkFramebuffer> frameBuffers;
        frameBuffers.resize(m_swapChainImages.size());

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

            VkResult result = vkCreateFramebuffer(m_device, &framebufferInfo, VK_NULL_HANDLE, &frameBuffers[i]);
            CHECK_VK_RESULT(result, "vkCreateFramebuffer");
            M4_LOG("Framebuffer[%d] Created Successfully",i);
        }
        return frameBuffers;
    }

    void VulkanCore::DestroyFrameBuffers(std::vector<VkFramebuffer> frameBuffers){
        for(int i=0; i<frameBuffers.size();i++){
            vkDestroyFramebuffer(m_device,frameBuffers[i], VK_NULL_HANDLE);
        }
    }

    BufferAndMemory VulkanCore::CreateVertexBuffer(const void* pVertices, size_t size){

        //1 create staging buffer
        VkBufferUsageFlags usage=VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags memoryProperties=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        BufferAndMemory stagingBam=CreateBuffer(size,usage,memoryProperties);

        //2 map the memory of  staging buffer
        void* pMappedMemory=VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        VkResult result=vkMapMemory(
            m_device,
            stagingBam.m_memory,
            offset,
            VK_WHOLE_SIZE,
            flags,
            &pMappedMemory
        );
        CHECK_VK_RESULT(result, "vkMapMemory - staging buffer");

        //3 copy verts to staging buffer
        memcpy(pMappedMemory,pVertices,size);

        //4 unmap
        vkUnmapMemory(m_device, stagingBam.m_memory);

        //5 create final buffer
        usage=VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        memoryProperties=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        BufferAndMemory bam=CreateBuffer(size,usage,memoryProperties);

        //6 copy staging buffer
        CopyBuffer(bam.m_buffer,stagingBam.m_buffer,size);
        
        //7 release resources
        stagingBam.Log("CreateVertexBuffer: stagingBam");
        stagingBam.Destroy(m_device);
        bam.Log("CreateVertexBuffer: bam");
        return bam;
    }

    void VulkanCore::GetFrameBufferSize(int&width, int& height) const
    {
        glfwGetWindowSize(m_pWindow,&width,&height);
    }

    void VulkanCore::CreateUniformBuffers(size_t size, std::vector<BufferAndMemory> &uniformBuffers )
    {

        // uniformBuffers.resize();

        for (int i=0; i< m_swapChainImages.size();i++)
        {
            uniformBuffers.push_back(CreateUniformBuffer(size));
            M4_LOG("create uniform buffer [%d]",i);
        }

    }

    BufferAndMemory VulkanCore::CreateUniformBuffer(size_t size){
        BufferAndMemory bam;
        VkBufferUsageFlags usage=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VkMemoryPropertyFlags props=VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        bam=CreateBuffer(size,usage,props);
        bam.Log("CreateUniformBuffer bam");
        return bam;
    }

    BufferAndMemory VulkanCore::CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo infoBuffer{};
        infoBuffer.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        infoBuffer.size=size;
        infoBuffer.usage=usage;
        infoBuffer.sharingMode=VK_SHARING_MODE_EXCLUSIVE;

        BufferAndMemory bam;

        VkResult result=vkCreateBuffer(m_device, &infoBuffer, VK_NULL_HANDLE, &bam.m_buffer);
        CHECK_VK_RESULT(result,"vkCreateBuffer");
        M4_LOG("buffer created.");

        VkMemoryRequirements memoryRequirements={};
        vkGetBufferMemoryRequirements(m_device, bam.m_buffer,&memoryRequirements);
        M4_LOG("buffer requires %d bytes", (int)memoryRequirements.size);

        uint32_t memoryTypeIndex= GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);
        M4_LOG("memory type index %d", memoryTypeIndex);

        VkMemoryAllocateInfo infoMemoryAllocate{};
        infoMemoryAllocate.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        infoMemoryAllocate.allocationSize=memoryRequirements.size;
        infoMemoryAllocate.memoryTypeIndex=memoryTypeIndex;

        result= vkAllocateMemory(m_device,&infoMemoryAllocate, VK_NULL_HANDLE, &bam.m_memory);
        CHECK_VK_RESULT(result,"vkAllocateMemory");

        result=vkBindBufferMemory(m_device,bam.m_buffer,bam.m_memory,0);
        CHECK_VK_RESULT(result, "vkBindBufferMemory")

        return bam;

    }

    void VulkanCore::CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size){

        //BeginCommandBuffer
        VkCommandBufferBeginInfo infoCommandBufferBegin = {};
        infoCommandBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        infoCommandBufferBegin.pNext = VK_NULL_HANDLE;
        infoCommandBufferBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        infoCommandBufferBegin.pInheritanceInfo = VK_NULL_HANDLE;

        VkResult result = vkBeginCommandBuffer(m_copyCommandBuffer, &infoCommandBufferBegin);
        CHECK_VK_RESULT(result, "vkBeginCommandBuffer");

        VkBufferCopy bufferCopy={};
        bufferCopy.srcOffset=0;
        bufferCopy.dstOffset=0;
        bufferCopy.size=size;
        vkCmdCopyBuffer(m_copyCommandBuffer, src,dst,1,&bufferCopy);

        vkEndCommandBuffer(m_copyCommandBuffer);

        m_queue.SubmitCommandBufferSync(m_copyCommandBuffer);
        m_queue.WaitIdle();
    }

    uint32_t VulkanCore::GetMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags requiredMemoryPropertyFlags){

        const VkPhysicalDeviceMemoryProperties& memoryProperties= m_physicalDevices.GetSelectedDevice().m_deviceMemoryProperties;

        for (uint32_t i =0; i<memoryProperties.memoryTypeCount;i++){
            if((memoryTypeBits & (1<<i))&&
            ((memoryProperties.memoryTypes[i].propertyFlags & requiredMemoryPropertyFlags)==requiredMemoryPropertyFlags)){
                return i;
            }
        }
        M4_LOG("no type for type %x requested memory Properties %x", memoryTypeBits, requiredMemoryPropertyFlags);
        exit(1);
        return -1;
    }

    void BufferAndMemory::Destroy(VkDevice device)
    {
        if(m_memory){
            vkFreeMemory( device,m_memory, VK_NULL_HANDLE);
        }
        if(m_buffer){
            vkDestroyBuffer(device,m_buffer,VK_NULL_HANDLE);
        }
    }
    
    void BufferAndMemory::Log(const char* tag)
    {
        if (!tag) tag = "";
        if (m_memory) {
            M4_LOG("BufferAndMemory(%s).m_memory = 0x%016llx", tag, (unsigned long long)m_memory);
        }
        if (m_buffer) {
            M4_LOG("BufferAndMemory(%s).m_buffer = 0x%016llx", tag, (unsigned long long)m_buffer);
        }
    }


    void BufferAndMemory::Update(VkDevice device, const void* pData, size_t size)
    {
        void * pMem=NULL;
        VkResult result=vkMapMemory(device,m_memory,0,size,0,&pMem);
        CHECK_VK_RESULT(result,"BufferAndMemory::Update - vkMapmemory")
        memcpy(pMem, pData,size);
        vkUnmapMemory(device, m_memory);

    }

    void VulkanTexture::Destroy(VkDevice Device)
    {
        if (m_sampler) {
            vkDestroySampler(Device, m_sampler, NULL);
        }
        
        if (m_imageView) {
            vkDestroyImageView(Device, m_imageView, NULL);
        }
        
        if (m_image) {
            vkDestroyImage(Device, m_image, NULL);
        }

        if (m_memory) {
            vkFreeMemory(Device, m_memory, NULL);
        }	
    }
}