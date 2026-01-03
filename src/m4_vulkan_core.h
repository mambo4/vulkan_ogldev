#pragma once
#define GLFW_INCLUDE_VULKAN
#include "m4_vulkan_utils.h"
#include "m4_vulkan_device.h"
#include "m4_vulkan_queue.h"
#include <vulkan/vulkan.h>
#include <glfw3.h>


namespace m4VK
{
    class BufferAndMemory {
    public:
        BufferAndMemory() {}

        VkBuffer m_buffer = NULL;
        VkDeviceMemory m_memory = NULL;
        VkDeviceSize m_allocationSize = 0;

        void Update(VkDevice Device, const void* pData, size_t Size);
        void Log(const char* tag);
        void Destroy(VkDevice Device);
    };

    class VulkanTexture{

        public:
            VulkanTexture(){}

            VkImage m_image=VK_NULL_HANDLE;
            VkDeviceMemory m_memory=VK_NULL_HANDLE;
            VkImageView m_imageView= VK_NULL_HANDLE;
            VkSampler m_sampler=VK_NULL_HANDLE;

            void Destroy(VkDevice Device);
    };

    class VulkanCore
    {
        public:
            VulkanCore();
            ~VulkanCore();

            void Init(const char* pAppName, GLFWwindow* pWindow);
            int GetSwapchainImageCount() const { return (int)m_swapChainImages.size(); }
            VkDevice GetDevice() const { return m_device; }//?need const?
            VkImage GetSwapchainImage(uint32_t imageIndex) const { return m_swapChainImages[imageIndex]; }
            VulkanQueue* GetQueue() { return &m_queue; }
            void CreateCommandBuffers(uint32_t commandBufferCount , VkCommandBuffer* pCommandBuffers);
            void FreeCommandBuffers(uint32_t commandBufferCount , VkCommandBuffer* pCommandBuffers);
            BufferAndMemory CreateVertexBuffer(const void* pVertices, size_t size);
            VkRenderPass CreateRenderPassSimple();
            std::vector<VkFramebuffer>CreateFrameBuffer(VkRenderPass renderPass);
            void DestroyFrameBuffers(std::vector<VkFramebuffer> frameBuffers);
            void CreateUniformBuffers(size_t size, std::vector<BufferAndMemory>&buffers);
            void GetFrameBufferSize(int&width, int& height)const;
            void CreateTexture(const char* pFilename, VulkanTexture& texture);

        private:
            void CreateInstance(const char* pAppName);
            void CreateDebugCallback();
            void CreateSurface();
            void CreateDevice();
            void CreateSwapChain();
            void CreateCommandBufferPool();
            uint32_t GetMemoryTypeIndex(uint32_t memTypeBits, VkMemoryPropertyFlags memPropFlags);
            void CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);
            BufferAndMemory CreateUniformBuffer(size_t size);
            BufferAndMemory CreateBuffer(VkDeviceSize size,VkBufferUsageFlags usage,VkMemoryPropertyFlags properties);
            void TransitionImageLayout(VkImage& image,VkFormat format,VkImageLayout oldLayout,VkImageLayout newLayout);
            void CopyBufferToImage(VkImage destination, VkBuffer source, uint32_t imageWidth,uint32_t imageHeight);
            void CreateImage(VulkanTexture& texture, uint32_t imageWidth,uint32_t imageHeight,VkFormat format,VkImageUsageFlags usage, VkMemoryPropertyFlagBits property, bool isCubemap);
            void CreateImageFromData(VulkanTexture& texture, const void* pPixels,uint32_t imageWidth,uint32_t imageHeight,VkFormat format, bool isCubemap);
            VkImageView CreateImageView( VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags imageViewAspectFlags, bool isCubemap);
            void UpdateTextureImage(VulkanTexture& texture,uint32_t imageWidth,uint32_t imageHeight,VkFormat textureFormat,const void* pPixels,bool isCubemap);
            void CreateTextureFromData(VulkanTexture& texture, const void* pPixels,uint32_t imageWidth,uint32_t imageHeight,VkFormat format, bool isCubemap);
            VkSampler CreateTextureSampler(VkDevice device, VkFilter minFilter, VkFilter maxFilter, VkSamplerAddressMode AddressMode);
            void ImageMemBarrier(VkCommandBuffer commandBuffer,VkImage image,VkFormat format,VkImageLayout oldLayout,VkImageLayout newLayout);
            VkInstance m_instance = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
            GLFWwindow* m_pWindow = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
            VulkanPhysicalDevices m_physicalDevices;    
            uint32_t m_queueFamilyIndex = 0;
            VkDevice m_device;
            VkSurfaceFormatKHR m_swapChainSurfaceFormat;
            VkSwapchainKHR m_swapChain   = VK_NULL_HANDLE;
            std::vector<VkImage> m_swapChainImages; 
            std::vector<VkImageView> m_swapChainImageViews;
            VkCommandPool m_commandBufferPool = VK_NULL_HANDLE;
            VulkanQueue m_queue;
            VkCommandBuffer m_copyCommandBuffer;
    
    };

}