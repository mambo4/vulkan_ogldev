#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "m4_vulkan_core.h"

namespace m4VK {

    struct SimpleMesh {
        BufferAndMemory m_bam;
        size_t m_vertexBufferSize = 0;
        VulkanTexture* m_pTexture=NULL;
        
        void Destroy(VkDevice device)
        {
            m_bam.Destroy(device);
            
            if (m_pTexture){
                m_pTexture->Destroy(device);
                delete m_pTexture;
            }
        }
    };

}