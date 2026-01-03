#include "m4_vulkan_pipeline.h"
#include "m4_vulkan_utils.h"
//lib
#include <glfw3.h>
//std
#include <array>
namespace m4VK{

    GraphicsPipeline::GraphicsPipeline(
        VkDevice device,
        GLFWwindow* pWindow,
        VkRenderPass renderPass,
        VkShaderModule shaderModule_vert,
        VkShaderModule shaderModule_frag,
        const SimpleMesh* pMesh,
        int imageCount,
        std::vector<BufferAndMemory>& uniformBuffers,
        int uniformSize,
        bool depthEnabled
    )
    {
        m_device=device;

        if(pMesh){
            CreateDescriptorSets(imageCount,pMesh, uniformBuffers,uniformSize);
        }

        VkPipelineShaderStageCreateInfo infoShaderStage[2]{};
        
        infoShaderStage[0].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        infoShaderStage[0].stage= VK_SHADER_STAGE_VERTEX_BIT;
        infoShaderStage[0].module = shaderModule_vert;
        infoShaderStage[0].pName="main";
        infoShaderStage[1].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        infoShaderStage[1].stage= VK_SHADER_STAGE_FRAGMENT_BIT;
        infoShaderStage[1].module = shaderModule_frag;
        infoShaderStage[1].pName="main";

        VkPipelineVertexInputStateCreateInfo infoVertInputState{};
        infoVertInputState.sType= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo infoInputAssemblyState{};
        infoInputAssemblyState.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        infoInputAssemblyState.topology= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        infoInputAssemblyState.primitiveRestartEnable= VK_FALSE;

        int windowWidth;
        int windowHeight;
        glfwGetWindowSize(pWindow,&windowWidth,&windowHeight);

        VkViewport viewport{};
        viewport.x=0;
        viewport.y=0;
        viewport.width=(float)windowWidth;
        viewport.height=(float)windowHeight;
        viewport.minDepth=0.0f;
        viewport.maxDepth=1.0f;

        VkRect2D scissor{};
        scissor.offset.x=0;
        scissor.offset.y=0;
        scissor.extent.width=(uint32_t)windowWidth;
        scissor.extent.height=(uint32_t)windowHeight;
        
        VkPipelineViewportStateCreateInfo infoViewportState{};
        infoViewportState.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        infoViewportState.viewportCount=1;
        infoViewportState.pViewports=&viewport;
        infoViewportState.scissorCount=1;
        infoViewportState.pScissors =&scissor;

        VkPipelineRasterizationStateCreateInfo infoRasterizationSate{};
        infoRasterizationSate.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        infoRasterizationSate.polygonMode=VK_POLYGON_MODE_FILL;
        infoRasterizationSate.cullMode=VK_CULL_MODE_NONE;
        infoRasterizationSate.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE;
        infoRasterizationSate.lineWidth=1.0f;

        VkPipelineMultisampleStateCreateInfo infoMultisampleSate{};
        infoMultisampleSate.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        infoMultisampleSate.rasterizationSamples =VK_SAMPLE_COUNT_1_BIT;
        infoMultisampleSate.sampleShadingEnable=VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.blendEnable=VK_FALSE;
        colorBlendAttachmentState.colorWriteMask=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo infoColorBlendSate{};
        infoColorBlendSate.sType= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        infoColorBlendSate.logicOpEnable=VK_FALSE;
        infoColorBlendSate.logicOp=VK_LOGIC_OP_COPY;
        infoColorBlendSate.attachmentCount=1;
        infoColorBlendSate.pAttachments=&colorBlendAttachmentState;

        VkPipelineLayoutCreateInfo infoPipelineLayout{};
        infoPipelineLayout.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        if(pMesh && pMesh->m_bam.m_buffer){
            infoPipelineLayout.setLayoutCount = 1;
            infoPipelineLayout.pSetLayouts = &m_descriptorSetLayout;
        }else{
            infoPipelineLayout.setLayoutCount = 0;
            infoPipelineLayout.pSetLayouts= VK_NULL_HANDLE;
        }

        VkResult result=vkCreatePipelineLayout(m_device,&infoPipelineLayout, VK_NULL_HANDLE, &m_pipelineLayout);
        CHECK_VK_RESULT(result,"vkCreatePipelineLayout");

        VkGraphicsPipelineCreateInfo infoGraphicsPipeline{};
        infoGraphicsPipeline.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        infoGraphicsPipeline.stageCount=ARRAY_COUNT(infoShaderStage);
        infoGraphicsPipeline.pStages=&infoShaderStage[0];
        infoGraphicsPipeline.pVertexInputState=&infoVertInputState;
        infoGraphicsPipeline.pInputAssemblyState=&infoInputAssemblyState;
        infoGraphicsPipeline.pViewportState=&infoViewportState;
        infoGraphicsPipeline.pRasterizationState=&infoRasterizationSate;
        infoGraphicsPipeline.pMultisampleState=&infoMultisampleSate;
        infoGraphicsPipeline.pColorBlendState=&infoColorBlendSate;
        infoGraphicsPipeline.layout=m_pipelineLayout;
        infoGraphicsPipeline.renderPass=renderPass;
        infoGraphicsPipeline.subpass=0;
        infoGraphicsPipeline.basePipelineHandle=VK_NULL_HANDLE;
        infoGraphicsPipeline.basePipelineIndex=-1;

        result= vkCreateGraphicsPipelines(m_device,VK_NULL_HANDLE,1,&infoGraphicsPipeline, VK_NULL_HANDLE, &m_pipeline);
        CHECK_VK_RESULT(result,"vkCreateGraphicsPipelines");
        M4_LOG("Graphics Pipeline created.");
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        vkDestroyDescriptorPool(m_device,m_descriptorPool, VK_NULL_HANDLE);
        vkDestroyDescriptorSetLayout(m_device,m_descriptorSetLayout,VK_NULL_HANDLE);
        vkDestroyPipelineLayout(m_device,m_pipelineLayout,VK_NULL_HANDLE);
        vkDestroyPipeline(m_device,m_pipeline,VK_NULL_HANDLE);
    };

    void GraphicsPipeline::bind(VkCommandBuffer buffer,int imageIndex)
    {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        if(m_descriptorSets.size()>0){
            vkCmdBindDescriptorSets(
                buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                0,//first descriptorSet
                1,//descriptorset count
                &m_descriptorSets[imageIndex],
                0,VK_NULL_HANDLE
            );
        }
    }

    void GraphicsPipeline::CreateDescriptorSets(int imageCount,const SimpleMesh* pMesh,std::vector<BufferAndMemory>& uniformBuffers,int uniformSize)
    {
        CreateDescriptorPool(imageCount);
        CreateDescriptorSetLayout(uniformBuffers,uniformSize,pMesh->m_pTexture);
        AllocateDescriptorSets(imageCount);
        UpdateDescriptorSets(imageCount,pMesh,uniformBuffers,uniformSize);
    }

    void GraphicsPipeline::CreateDescriptorPool(int imageCount)
    {
        VkDescriptorPoolSize poolSize[3]{};
        poolSize[0].descriptorCount=(uint32_t)imageCount;
        poolSize[0].type=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSize[1].descriptorCount=(uint32_t)imageCount;
        poolSize[1].type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize[2].descriptorCount=(uint32_t)imageCount;
        poolSize[2].type=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        VkDescriptorPoolCreateInfo infoDescriptorPool{};
        infoDescriptorPool.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        infoDescriptorPool.flags=0;
        infoDescriptorPool.maxSets=(uint32_t)imageCount;
        infoDescriptorPool.poolSizeCount=3;
        infoDescriptorPool.pPoolSizes=poolSize;

        VkResult result=vkCreateDescriptorPool(m_device, &infoDescriptorPool,VK_NULL_HANDLE,&m_descriptorPool);
        CHECK_VK_RESULT(result,"vkCreateDescriptorPool");
        M4_LOG("descriptor pool created");

    }

    void GraphicsPipeline::CreateDescriptorSetLayout(
        std::vector<BufferAndMemory>& uniformBuffers,
        int UniformDataSize,
        VulkanTexture* pTexture) 
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        
        VkDescriptorSetLayoutBinding vertexShaderLayoutBinding{};
        vertexShaderLayoutBinding.binding=0;
        vertexShaderLayoutBinding.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        vertexShaderLayoutBinding.descriptorCount=1;
        vertexShaderLayoutBinding.stageFlags=VK_SHADER_STAGE_VERTEX_BIT;

        layoutBindings.push_back(vertexShaderLayoutBinding);

        VkDescriptorSetLayoutBinding vertexShaderLayoutBinding_uniform{};
        vertexShaderLayoutBinding_uniform.binding=1;
        vertexShaderLayoutBinding_uniform.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vertexShaderLayoutBinding_uniform.descriptorCount=1;
        vertexShaderLayoutBinding_uniform.stageFlags=VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding fragmentShaderLayoutBinding{};
        fragmentShaderLayoutBinding.binding=2;
        fragmentShaderLayoutBinding.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        fragmentShaderLayoutBinding.descriptorCount=1;
        fragmentShaderLayoutBinding.stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT;


        if( uniformBuffers.size()>0){
            layoutBindings.push_back(vertexShaderLayoutBinding_uniform);
        };

        if(pTexture){
            layoutBindings.push_back(fragmentShaderLayoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo infoDescriptorSetLayout{};
        infoDescriptorSetLayout.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        infoDescriptorSetLayout.pNext=VK_NULL_HANDLE;
        infoDescriptorSetLayout.bindingCount=(uint32_t)layoutBindings.size();
        infoDescriptorSetLayout.pBindings=layoutBindings.data();

        VkResult result=vkCreateDescriptorSetLayout(m_device, &infoDescriptorSetLayout, VK_NULL_HANDLE, &m_descriptorSetLayout);
        CHECK_VK_RESULT(result,"vkCreateDescriptorSetLayout");
        M4_LOG("CreateDescriptorSetLayout");

    }

    void GraphicsPipeline::AllocateDescriptorSets(int imageCount)
    {

        std::vector<VkDescriptorSetLayout> layouts(imageCount, m_descriptorSetLayout);

        VkDescriptorSetAllocateInfo infoDescriptorSetAllocate{};
        infoDescriptorSetAllocate.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        infoDescriptorSetAllocate.pNext=VK_NULL_HANDLE;
        infoDescriptorSetAllocate.descriptorPool=m_descriptorPool;
        infoDescriptorSetAllocate.descriptorSetCount=(uint32_t)imageCount;
        infoDescriptorSetAllocate.pSetLayouts=layouts.data();

        m_descriptorSets.resize(imageCount);

        VkResult result=vkAllocateDescriptorSets(m_device,&infoDescriptorSetAllocate, m_descriptorSets.data());
        CHECK_VK_RESULT(result,"vkAllocateDescriptorSets");
        M4_LOG("AllocateDescriptorSets");
    }

    void GraphicsPipeline::UpdateDescriptorSets(int imageCount, const SimpleMesh* pMesh,std::vector<BufferAndMemory>& uniformBuffers, int uniformSize)
    {
        VkDescriptorBufferInfo infoDescriptorBuffer{};
        infoDescriptorBuffer.buffer=pMesh->m_bam.m_buffer;
        infoDescriptorBuffer.offset=0;
        infoDescriptorBuffer.range=pMesh->m_vertexBufferSize;

        VkDescriptorImageInfo imageInfo;

        if(pMesh->m_pTexture){
            imageInfo.sampler=pMesh->m_pTexture->m_sampler;
            imageInfo.imageView=pMesh->m_pTexture->m_imageView;
            imageInfo.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        std::vector<VkWriteDescriptorSet> writeDescriptorSets;
        for ( size_t i=0; i<imageCount; i++)
        {
            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet=m_descriptorSets[i];
            writeDescriptorSet.dstBinding=0;
            writeDescriptorSet.dstArrayElement=0;
            writeDescriptorSet.descriptorCount=1;
            writeDescriptorSet.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSet.pBufferInfo = &infoDescriptorBuffer;

            writeDescriptorSets.push_back(writeDescriptorSet);

            if (uniformBuffers.size()>0){   
                VkDescriptorBufferInfo buffer_info_uniform{};
                buffer_info_uniform.buffer=uniformBuffers[i].m_buffer;
                buffer_info_uniform.offset=0;
                buffer_info_uniform.range=(VkDeviceSize)uniformSize;

                VkWriteDescriptorSet writeDescriptorSet_buffer{};
                writeDescriptorSet_buffer.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet_buffer.dstSet=m_descriptorSets[i];
                writeDescriptorSet_buffer.dstBinding=1;
                writeDescriptorSet_buffer.dstArrayElement=0;
                writeDescriptorSet_buffer.descriptorCount=1;
                writeDescriptorSet_buffer.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptorSet_buffer.pBufferInfo = &buffer_info_uniform;

                writeDescriptorSets.push_back(writeDescriptorSet_buffer);

            }

            if(pMesh->m_pTexture){
                VkWriteDescriptorSet writeDescriptorSet_texture{};
                writeDescriptorSet_texture.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet_texture.dstSet=m_descriptorSets[i];
                writeDescriptorSet_texture.dstBinding=2;
                writeDescriptorSet_texture.dstArrayElement=0;
                writeDescriptorSet_texture.descriptorCount=1;
                writeDescriptorSet_texture.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptorSet_texture.pImageInfo=&imageInfo;

                writeDescriptorSets.push_back(writeDescriptorSet_texture);
            }

            vkUpdateDescriptorSets(m_device,(uint32_t)writeDescriptorSets.size(),writeDescriptorSets.data(),0,VK_NULL_HANDLE);
            M4_LOG("UpdateDescriptorSets");
        }
    }
} 