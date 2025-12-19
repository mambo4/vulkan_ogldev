#include "m4_vulkan_utils.h"
#include <vulkan/vulkan.h>
#include <iostream>

    const char* m4VK::GetDebugSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT Severity)
    {
        switch (Severity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                return "VERBOSE";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                return "INFO";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                return "WARNING";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }

    const char* m4VK::GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type)
    {
        switch (Type)
        {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                return "GENERAL";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                return "VALIDATION";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                return "PERFORMANCE";
            default:
                return "UNKNOWN";
        }
    }     

    const char* m4VK::GetObjectTypeString(VkObjectType type)
    {
        switch (type)
        {
            case VK_OBJECT_TYPE_INSTANCE:
                return "INSTANCE";
            case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
                return "PHYSICAL_DEVICE";
            case VK_OBJECT_TYPE_DEVICE:
                return "DEVICE";
            case VK_OBJECT_TYPE_QUEUE:
                return "QUEUE";
            case VK_OBJECT_TYPE_SEMAPHORE:
                return "SEMAPHORE";
            case VK_OBJECT_TYPE_COMMAND_BUFFER:
                return "COMMAND_BUFFER";
            // Add more cases as needed
            default:
                return "UNKNOWN_OBJECT_TYPE";
        }
    }
