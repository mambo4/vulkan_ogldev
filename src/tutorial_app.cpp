#include "m4_vulkan_core.h"
#include "m4_vulkan_shader.h"
#include "m4_vulkan_pipeline.h"
#include "colors.h"
#include "m4_vulkan_simple_mesh.h"

//std
#include <stdio.h>
#include <stdlib.h>

//libs
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include "glm.hpp"
#include "ext.hpp"

#define WINDOW_WIDTH 1280      
#define WINDOW_HEIGHT 720



#define APP_NAME "OGL_tutorial_17: uniform buffers"

GLFWwindow* window=NULL;

void GLFW_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

//todo: mouse callback

class VulkanApp
{
    public:
        VulkanApp() {}
        ~VulkanApp() {
            m_vkCore.FreeCommandBuffers((uint32_t)m_commandBuffers.size(), m_commandBuffers.data());
            m_vkCore.DestroyFrameBuffers(m_frameBuffers);
            vkDestroyShaderModule(m_vkCore.GetDevice(),m_shaderModule_vert, VK_NULL_HANDLE);
            vkDestroyShaderModule(m_vkCore.GetDevice(),m_shaderModule_frag, VK_NULL_HANDLE);
            delete m_pPipeline;
            vkDestroyRenderPass(m_vkCore.GetDevice(), m_renderPass, VK_NULL_HANDLE);
            m_mesh.Destroy(m_device);

            for (int i = 0; i < m_uniformBuffers.size(); i++) {
			    m_uniformBuffers[i].Destroy(m_device);
		    }
        }

        void Init(const char* pAppName, GLFWwindow* pWindow) 
        {
            m_pWindow=pWindow;
            m_vkCore.Init(pAppName, pWindow);
            m_numSwapchainImages = m_vkCore.GetSwapchainImageCount();
            m_device=m_vkCore.GetDevice();
            m_pQueue = m_vkCore.GetQueue();
            m_renderPass=m_vkCore.CreateRenderPassSimple();
            m_frameBuffers=m_vkCore.CreateFrameBuffer(m_renderPass);
            CreateShaders();
            CreateVertexBuffer();
            CreateUniformBuffers();
            CreatePipeline();
            CreateCommandBuffers();
            RecordCommandBuffers();
        }
        
        void RenderScene()
        {
            uint32_t imageIndex = m_pQueue->AcquireNextImage();
            UpdateUniformBuffers(imageIndex);
            m_pQueue->SubmitCommandBufferAsync(m_commandBuffers[imageIndex]);
            m_pQueue->PresentImage(imageIndex); 
        }

    private:
        void CreateShaders()
        {
            m_shaderModule_vert=m4VK::createShaderModuleFromText(m_device,"../shaders/test.vert");
            m_shaderModule_frag=m4VK::createShaderModuleFromText(m_device,"../shaders/test.frag");
        }

        void CreateVertexBuffer(){

            struct Vertex
            {
                Vertex(const glm::vec3&p, const glm::vec2 t)
                {
                    pos=p;
                    uv=t;
                }
                glm::vec3 pos;
                glm::vec2 uv;

            };

            std::vector<Vertex>vertices ={
                Vertex({-0.5f,-0.5f,0.0f},{0.2f,0.8f}),
                Vertex({0.5f,-0.5f,0.0f},{0.8f,0.8f}),
                Vertex({0.0f,0.5f,0.0f},{0.5f,0.2f})
            };

            m_mesh.m_vertexBufferSize=sizeof(vertices[0])*vertices.size();
            m_mesh.m_bam=m_vkCore.CreateVertexBuffer(vertices.data(),m_mesh.m_vertexBufferSize );
            

        }

        struct UniformData {
            glm::mat4 WVP;
        };

        void CreateUniformBuffers()
        {
            m_vkCore.CreateUniformBuffers(sizeof(UniformData), m_uniformBuffers);//TODO: how to pass &m_uniformBuffers without lvalue complaints
        }

        void CreateCommandBuffers()
        {
            m_commandBuffers.resize(m_numSwapchainImages);
            m_vkCore.CreateCommandBuffers(m_numSwapchainImages, m_commandBuffers.data());
            M4_LOG("CreateCommandBuffers...");
        }


        void CreatePipeline(){

            m_pPipeline = new m4VK::GraphicsPipeline(
                m_device,
                m_pWindow,
                m_renderPass,
                m_shaderModule_vert,
                m_shaderModule_frag,
                &m_mesh,
                m_numSwapchainImages,
                m_uniformBuffers,
                sizeof(UniformData)
            );
        }

        void RecordCommandBuffers()
        {

            VkClearColorValue clearColor = COLOR_BLACK;
            VkClearValue clearValue;
            clearValue.color = clearColor;

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = VK_NULL_HANDLE;
            renderPassBeginInfo.renderPass = m_renderPass;
            renderPassBeginInfo.renderArea.offset.x = 0;
            renderPassBeginInfo.renderArea.offset.y = 0;
            renderPassBeginInfo.renderArea.extent.width = WINDOW_WIDTH;
            renderPassBeginInfo.renderArea.extent.height = WINDOW_HEIGHT;
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = &clearValue;



            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

 
            for (uint32_t i = 0; i < m_commandBuffers.size(); i++)
            {

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pNext = VK_NULL_HANDLE;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

                VkResult result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);
                CHECK_VK_RESULT(result, "vkBeginCommandBuffer");

                renderPassBeginInfo.framebuffer = m_frameBuffers[i];
                vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                
                m_pPipeline->bind(m_commandBuffers[i],i);

                 uint32_t vertCount =3;
                 uint32_t instanceCount=1;
                 uint32_t firstVertex=0;
                 uint32_t firstInstance=0;

                vkCmdDraw(m_commandBuffers[i],vertCount, instanceCount, firstVertex, firstInstance);  

                vkCmdEndRenderPass(m_commandBuffers[i]);

                result = vkEndCommandBuffer(m_commandBuffers[i]);
                CHECK_VK_RESULT(result, "vkEndCommandBuffer");
            }

            M4_LOG("RecordCommandBuffers...");
        }

        void UpdateUniformBuffers(uint32_t imageIndex)
        {
            glm::mat4 WVP=glm::mat4(1.0);
            m_uniformBuffers[imageIndex].Update(m_device,&WVP, sizeof(WVP));
        }

        GLFWwindow* m_pWindow=VK_NULL_HANDLE;
        m4VK::VulkanCore m_vkCore;
        m4VK::VulkanQueue* m_pQueue = VK_NULL_HANDLE;
        VkDevice m_device= VK_NULL_HANDLE;
        int m_numSwapchainImages = 0;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VkFramebuffer> m_frameBuffers;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkShaderModule m_shaderModule_vert;
        VkShaderModule m_shaderModule_frag;
        m4VK::GraphicsPipeline* m_pPipeline = NULL;
        m4VK::SimpleMesh m_mesh;
        std::vector<m4VK::BufferAndMemory> m_uniformBuffers;
        //GLMCameraFirstPerson* m_gameCamera=NULL; //CONTINUE
        int m_windowWidth=0;
        int m_windowHeight=0;
};

int main(int argc, char* argv[])
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Non-resizable window

    VulkanApp App;
     


    GLFWwindow* pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, NULL, NULL);

    if (!pWindow)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetKeyCallback(pWindow, GLFW_Key_Callback);

    App.Init(APP_NAME,pWindow);

    while (!glfwWindowShouldClose(pWindow))
    {
        App.RenderScene();
        glfwPollEvents();
    }

    glfwDestroyWindow(pWindow);
    glfwTerminate();
    return EXIT_SUCCESS;
}