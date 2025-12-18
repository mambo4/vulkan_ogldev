#pragma once

#include <vulkan/vulkan.h>


#define CHECK_VK_RESULT(value, message) if (value != VK_SUCCESS) {\
     fprintf(stderr, "Error %s[%d]: %s(%x)\n", __FILE__, __LINE__, message, value);\
     exit(1); }    

namespace m4VK
{
    class VulkanCore
    {
        public:
            VulkanCore();
            ~VulkanCore();

            void Init(const char* pAppName);


        private:
            void CreateInstance(const char* pAppName);
            VkInstance m_instance=nullptr;
    };

}