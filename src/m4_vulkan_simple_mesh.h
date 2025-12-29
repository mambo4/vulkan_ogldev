#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "m4_vulkan_core.h"

namespace m4VK {

    struct SimpleMesh {
        BufferAndMemory m_bam=BufferAndMemory();
        size_t m_vertexBufferSize = 0;

        void Destroy(VkDevice device)
        {
            m_bam.Destroy(device);
        }
    };

}