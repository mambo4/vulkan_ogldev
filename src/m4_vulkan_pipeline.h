#pragma once
//#include "m4_vulkan_simple_mesh.h"
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include<vector>
namespace m4VK{


    class GraphicsPipeline {

        public: 

            GraphicsPipeline(
                VkDevice device,
                GLFWwindow* pWindow,
                VkRenderPass renderPass,
                VkShaderModule shaderModule_vert,
                VkShaderModule shaderModule_frag,
                VkBuffer vertexBuffer,
                size_t vertexBufferSize,
                int imageCount
            );

            ~GraphicsPipeline();
            void bind(VkCommandBuffer buffer, int imageIndex);
        
        private:
            void CreateDescriptorPool(int imageCount);
	        void CreateDescriptorSet(int imageCount, const VkBuffer& vertexBuffer, size_t vertexBufferSize);
            //void CreateDescriptorSets(int imageCount, const SimpleMesh* pMesh);
            VkDevice m_device;
            VkPipeline m_pipeline;
            VkPipelineLayout m_pipelineLayout;
            VkDescriptorPool m_descriptorPool;
            VkDescriptorSetLayout m_decscriptorSetLayout;
            std::vector<VkDescriptorSet> m_descriptorSets;
    };

}