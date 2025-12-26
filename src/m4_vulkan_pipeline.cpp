#include "m4_vulkan_pipeline.h"
#include "m4_vulkan_utils.h"
#include <glfw3.h>

namespace m4VK{

    GraphicsPipeline::GraphicsPipeline(
        VkDevice device,
        GLFWwindow* pWindow,
        VkRenderPass renderPass,
        VkShaderModule shaderModule_vert,
        VkShaderModule shaderModule_frag)
    {
        m_device=device;

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
        infoPipelineLayout.setLayoutCount=0;
        infoPipelineLayout.pSetLayouts=VK_NULL_HANDLE;

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


    GraphicsPipeline::~GraphicsPipeline(){
        vkDestroyPipelineLayout(m_device,m_pipelineLayout,VK_NULL_HANDLE);
        vkDestroyPipeline(m_device,m_pipeline,VK_NULL_HANDLE);
    };

    void GraphicsPipeline::bind(VkCommandBuffer buffer){
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

} 