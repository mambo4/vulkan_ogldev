#include "m4_vulkan_core.h"
#include "m4_vulkan_shader.h"
#include "m4_vulkan_pipeline.h"
#include "colors.h"
#include "m4_vulkan_simple_mesh.h"
#include "m4_vulkan_glfw.h"
#include "m4_glm_camera.h"

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

#define TEXTURE_00 "../assets/texture/purple_brick.jpg"
#define VERT_SHADER "../shaders/test.vert"
#define FRAG_SHADER "../shaders/test.frag"

#define APP_NAME "OGL_tutorial_17: uniform buffers"

// GLFWwindow* window=NULL;

// void GLFW_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
// {
//     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, GLFW_TRUE);
// }


class VulkanApp: public m4VK::GLFWCallbacks
{
    public:
        VulkanApp(int width, int height) 
        {
            m_windowWidth=width;
            m_windowHeight=height;
        }

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

        void Init(const char* pAppName) 
        {
            m_pWindow=m4VK::glfw_vulkan_init(WINDOW_WIDTH,WINDOW_HEIGHT,pAppName);
            m_vkCore.Init(pAppName, m_pWindow);
            m_numSwapchainImages = m_vkCore.GetSwapchainImageCount();
            m_device=m_vkCore.GetDevice();
            m_pQueue = m_vkCore.GetQueue();
            m_renderPass=m_vkCore.CreateRenderPassSimple();
            m_frameBuffers=m_vkCore.CreateFrameBuffer(m_renderPass);
            CreateShaders();
            CreateMesh();
            CreateUniformBuffers();
            CreatePipeline();
            CreateCommandBuffers();
            RecordCommandBuffers();
            DefaultCreateCameraPers();
            m4VK::glfw_vulkan_set_callbacks(m_pWindow, this);
        }
        
        void RenderScene()
        {
            uint32_t imageIndex = m_pQueue->AcquireNextImage();
            UpdateUniformBuffers(imageIndex);
            m_pQueue->SubmitCommandBufferAsync(m_commandBuffers[imageIndex]);
            m_pQueue->PresentImage(imageIndex); 
        }

        void Key(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
        {
            bool handled=true;

            switch(key)
            {
                case GLFW_KEY_ESCAPE:
                case GLFW_KEY_Q:
                    glfwDestroyWindow(m_pWindow);
                    glfwTerminate();
                    exit(0);
                default:
                    handled=false;
            }

            if(!handled)
            {
                handled=GLFWCameraHandler(m_pGameCamera->m_movement, key, action, mods);
            }
        }

        void MouseMove(GLFWwindow* pWindow, double  xpos, double ypos)
        {
            m_pGameCamera->SetMousePos( (float)xpos,(float)ypos );
        }

        void MouseButton(GLFWwindow* pWindow, int button, int action, int mods)
        {
            m_pGameCamera->HandleMouseButton(button, action, mods);
        }
        
        void ExecuteMainLoop()
        {
            float time=(float)glfwGetTime();

            while(!glfwWindowShouldClose(m_pWindow))
            {
                float timeNow=(float)glfwGetTime();
                float dt=timeNow-time;
                m_pGameCamera->Update(dt);
                RenderScene();
                time=timeNow;
                glfwPollEvents();
            }
 
            glfwTerminate();
        }

    private:
        void CreateShaders()
        {
            m_shaderModule_vert=m4VK::createShaderModuleFromText(m_device,VERT_SHADER);
            m_shaderModule_frag=m4VK::createShaderModuleFromText(m_device,FRAG_SHADER);
        }


        void CreateMesh()
        {
            CreateVertexBuffer();
            LoadTexture();
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
                Vertex({-0.5f,-0.5f,0.0f},{0.0f,0.0f}),
                Vertex({0.5f,-0.5f,0.0f},{0.0f,1.0f}),
                Vertex({-0.5f,0.5f,0.0f},{1.0f,0.0f}),
                Vertex({-0.5f,0.5f,0.0f},{1.0f,0.0f}),
                Vertex({0.5f,-0.5f,0.0f},{0.0f,1.0f}),
                Vertex({0.5f,0.5f,0.0f},{1.0f,1.0f}),
            };

            m_mesh.m_vertexBufferSize=sizeof(vertices[0])*vertices.size();
            m_mesh.m_bam=m_vkCore.CreateVertexBuffer(vertices.data(),m_mesh.m_vertexBufferSize );
            

        }

        void LoadTexture()
        {
            m_mesh.m_pTexture = new m4VK::VulkanTexture;
            m_vkCore.CreateTexture(TEXTURE_00,*(m_mesh.m_pTexture));            
        }

        struct UniformData {
            glm::mat4 WVP;
        };

        void CreateUniformBuffers()
        {
            m_vkCore.CreateUniformBuffers(sizeof(UniformData), m_uniformBuffers);//TODO: how to pass &m_uniformBuffers without lvalue complaints
        }

        void DefaultCreateCameraPers()
        {
            float FOV=45.0f;
            float zNear=0.1f;
            float zFar=1000.0f;

            DefaultCreateCameraPers(FOV,zNear,zFar);

        }

        void DefaultCreateCameraPers(float FOV, float zNear,float zFar)
        {

            if ((m_windowWidth==0)||(m_windowHeight==0))
            {
                printf("invalid window dimension w: %d ,. h:%d",m_windowWidth,m_windowHeight);
                exit(1);
            }

            if (m_pGameCamera)
            {
                printf("camera already initilaized");
                exit(1);
            }

            PespectiveProjectionInfo projectionInfo ={FOV,(float)m_windowWidth,(float)m_windowHeight, zNear,zFar};
            
            glm::vec3 pos(0.0f,0.0f,-5.0f);
            glm::vec3 target(0.0f,0.0f,1.0f);
            glm::vec3 up(0.0f,1.0f,0.0f);

            m_pGameCamera= new GLMCameraFirstPerson(pos, target,up,projectionInfo );

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
                sizeof(UniformData),
                false
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

                 uint32_t vertCount =6;
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
            static float theta =0.0f;
            glm::mat4 rotateMatrix=glm::mat4(1.0);
            rotateMatrix=glm::rotate(rotateMatrix, glm::radians(theta), glm::normalize(glm::vec3(0.0f,0.0f,1.0f)));
            theta+=0.01f;
            glm::mat4 VP=m_pGameCamera->GetVPMatrix();
            glm::mat4 WVP=VP*rotateMatrix;
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
        GLMCameraFirstPerson* m_pGameCamera=NULL;
        int m_windowWidth=0;
        int m_windowHeight=0;
};

int main(int argc, char* argv[])
{
   VulkanApp App(WINDOW_WIDTH,WINDOW_HEIGHT);
   App.Init(APP_NAME);
   App.ExecuteMainLoop();
}