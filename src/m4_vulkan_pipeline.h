#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

namespace m4VK{


    class GraphicsPipeline {

        public: 

            GraphicsPipeline(
                VkDevice device,
                GLFWwindow* pWindow,
                VkRenderPass renderPass,
                VkShaderModule shaderModule_vert,
                VkShaderModule shaderModule_frag
            );

            ~GraphicsPipeline();
            void bind(VkCommandBuffer buffer);
        
        private:

            VkDevice m_device;
            VkPipeline m_pipeline;
            VkPipelineLayout m_pipelineLayout;
    };

}