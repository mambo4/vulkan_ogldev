#pragma once
#include <vulkan/vulkan.h>
#include <stdio.h>



#define CHECK_VK_RESULT(value, message) if (value != VK_SUCCESS) {\
     fprintf(stderr, "Error %s[%d]: %s(%x)\n", __FILE__, __LINE__, message, value);\
     exit(1); }

#define M4_ERROR(fmt, ...) fprintf(stderr, "ERROR: " fmt " (%s:[%d])\n", ##__VA_ARGS__, __FILE__, __LINE__)
#define M4_LOG(fmt, ...)   fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#define M4_DEBUG(fmt, ...)   fprintf(stdout, fmt " (%s:[%d])\n", ##__VA_ARGS__, __FILE__, __LINE__)

namespace m4VK {

    const char* GetDebugSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT Severity);

    const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type);  

    const char* GetObjectTypeString(VkObjectType type);
}