#pragma once
#include <vulkan/vulkan.h>

namespace m4VK{

    VkShaderModule createShaderModuleFromText(VkDevice& device, const char* pFilename);

    VkShaderModule createShaderModuleFromBinary(VkDevice& device, const char* pFilename);

}