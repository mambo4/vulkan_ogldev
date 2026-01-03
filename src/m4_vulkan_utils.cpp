#include "m4_vulkan_utils.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <assert.h>

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


    bool m4VK::readFileText(const char* pFileName, std::string& outFile)//totally vibe coded here lets see if it works
    {
        std::ifstream file(pFileName);
        if (!file.is_open()) {
            M4_ERROR("Failed to open file: %s", pFileName);
            return false;
        }

        outFile.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

        file.close();
        return true;
    }

    char*  m4VK::readFileBinary(const char* pFileName, int& size)
    {
        FILE* f = NULL;

        errno_t err = fopen_s(&f, pFileName, "rb");

        if (!f) {
            char buf[256] = { 0 };
            strerror_s(buf, sizeof(buf), err);
            M4_ERROR("Error opening '%s': %s\n", pFileName, buf);
            exit(0);
        }

        struct stat stat_buf;
        int error = stat(pFileName, &stat_buf);

        if (error) {
            char buf[256] = { 0 };
            strerror_s(buf, sizeof(buf), err);
            M4_ERROR("Error getting file stats: %s\n", buf);
            return NULL;
        }

        size = stat_buf.st_size;

        char* p = (char*)malloc(size);
        assert(p);

        size_t bytes_read = fread(p, 1, size, f);

        if (bytes_read != size) {
            char buf[256] = { 0 };
            strerror_s(buf, sizeof(buf), err);
            M4_ERROR("Read file error file: %s\n", buf);
            exit(0);
        }

        fclose(f);

        return p;

    }

    void m4VK::writeFileBinary(const char* pFileName, const void* pData, int size)
    {
        std::ofstream file(pFileName, std::ios::binary);
        if (!file.is_open()) {
            M4_ERROR("Failed to open file for writing: %s", pFileName);
            exit(0);
        }

        file.write(static_cast<const char*>(pData), size);
        file.close();
    }

int m4VK::GetBytesPerPixel(VkFormat format)
{
    //todo: implement for multiple formats
    int bytes=4;
    M4_LOG("GetBytesPerTexFormat() always returns 4 bytes per pixel. Format: (%d)",format);
    return ( bytes);

}