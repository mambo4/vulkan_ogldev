#include "m4_vulkan_shader.h"
#include "m4_vulkan_utils.h"

//vulkan 
#include <vulkan/vulkan.h>
#include "glslang/Include/glslang_c_interface.h"
#include "glslang/Public/resource_limits_c.h"

//std
#include <stdio.h>
#include<vector>
#include<string>
#include <assert.h>


namespace m4VK{

    struct ShaderStruct{
        std::vector<uint32_t> SPIRV;// may need unsigned int?
        VkShaderModule shaderModule=VK_NULL_HANDLE;
    };

    static void printShaderSource(const char* shaderSource){
        int line =1;
        printf("\n(%3i) ", line);

        while(shaderSource && *shaderSource++){
            if(*shaderSource == '\n'){
                printf("\n(%3i) ", ++line);
            }else if (*shaderSource == '\r'){
                //do nothing
            }else{
                printf("%c", *shaderSource);
            }
        }
    }

    static size_t compileShader( VkDevice& device, glslang_stage_t stage, const char* shaderSource, ShaderStruct& shaderStruct){

        glslang_input_t input{};
        input.language = GLSLANG_SOURCE_GLSL;
        input.stage = stage;

        input.client = GLSLANG_CLIENT_VULKAN;
        input.client_version = GLSLANG_TARGET_VULKAN_1_1;
        input.target_language = GLSLANG_TARGET_SPV;
        input.target_language_version = GLSLANG_TARGET_SPV_1_3;
        input.code = shaderSource;
        input.default_version = 100;
        input.default_profile = GLSLANG_NO_PROFILE;
        input.force_default_version_and_profile = false;
        input.forward_compatible = false;
        input.messages = GLSLANG_MSG_DEFAULT_BIT;
        input.resource = glslang_default_resource();


        glslang_shader_t* shader = glslang_shader_create(&input);

        if(!glslang_shader_preprocess(shader,&input)){
            fprintf(stderr,"Failed to create shader\n");
            fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
		    fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
            printShaderSource(shaderSource);
            return 0;
        }

        if (!glslang_shader_parse(shader, &input)) {
            fprintf(stderr, "GLSL parsing failed\n");
            fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
            fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
            printShaderSource(shaderSource);
            return 0;
        
        }

        glslang_program_t* program = glslang_program_create();
        glslang_program_add_shader(program, shader);

        if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT|GLSLANG_MSG_VULKAN_RULES_BIT)) {
            fprintf(stderr, "GLSL linking failed\n");
            fprintf(stderr, "\n%s", glslang_program_get_info_log(program));
            fprintf(stderr, "\n%s", glslang_program_get_info_debug_log(program));
            return 0;
        }

        glslang_program_SPIRV_generate(program, stage);

        shaderStruct.SPIRV.resize(glslang_program_SPIRV_get_size(program));

        glslang_program_SPIRV_get(program, shaderStruct.SPIRV.data());

        const char* spirv_messages=glslang_program_SPIRV_get_messages(program);
        if(spirv_messages){
            printf("\n%s\n",spirv_messages);
        }   

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; 
        createInfo.codeSize = shaderStruct.SPIRV.size() * sizeof(uint32_t);
        createInfo.pCode = shaderStruct.SPIRV.data();
        VkResult result = vkCreateShaderModule(device, &createInfo, VK_NULL_HANDLE, &shaderStruct.shaderModule);
        CHECK_VK_RESULT(result, "vkCreateShaderModule: Failed to create shader module");

        glslang_program_delete(program);
        glslang_shader_delete(shader);

        bool success = shaderStruct.SPIRV.size()>0;
        return success;
    }
    
    static glslang_stage_t ShaderStageFromFilename(const char* pFilename)
    {
        std::string s(pFilename);

        if (s.ends_with(".vert")) {
            return GLSLANG_STAGE_VERTEX;
        }

        if (s.ends_with(".frag")) {
            return GLSLANG_STAGE_FRAGMENT;
        }

        if (s.ends_with(".geom")) {
            return GLSLANG_STAGE_GEOMETRY;
        }

        if (s.ends_with(".comp")) {
            return GLSLANG_STAGE_COMPUTE;
        }

        if (s.ends_with(".tesc")) {
            return GLSLANG_STAGE_TESSCONTROL;
        }

        if (s.ends_with(".tese")) {
            return GLSLANG_STAGE_TESSEVALUATION;
        }

        printf("Unknown shader stage in '%s'\n", pFilename);
        exit(1);

        return GLSLANG_STAGE_VERTEX;
    }

    VkShaderModule createShaderModuleFromText(VkDevice& device, const char* pFilename){
        std::string shaderSource;
        if(!m4VK::readFileText(pFilename, shaderSource)){
            fprintf(stderr, "Failed to read shader file '%s'\n", pFilename);
            exit(1);
        }

        glslang_initialize_process();
        ShaderStruct shaderStruct;
        glslang_stage_t stage = ShaderStageFromFilename(pFilename);

        if(!compileShader(device, stage, shaderSource.c_str(), shaderStruct)){
            fprintf(stderr, "Failed to compile shader file '%s'\n", pFilename);
            exit(1);
        }

        std::string compiledFilename = std::string(pFilename) + ".spv";
        m4VK::writeFileBinary(compiledFilename.c_str(), shaderStruct.SPIRV.data(), static_cast<int>(shaderStruct.SPIRV.size() * sizeof(uint32_t))); // may bnot need* sizeof(uint32_t)

        glslang_finalize_process();

        return shaderStruct.shaderModule;
    }

    VkShaderModule createShaderModuleFromBinary(VkDevice& device, const char* pFileName){

        int codeSize=0;
        char* pShaderCode=m4VK::readFileBinary(pFileName, codeSize);
        assert(pShaderCode);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType =VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize =(size_t)codeSize;
        createInfo.pCode = (const uint32_t*) pShaderCode;

        VkShaderModule shaderModule;

        VkResult result = vkCreateShaderModule(device,&createInfo, VK_NULL_HANDLE, &shaderModule);
        CHECK_VK_RESULT(result,"vkCreateShaderModule");
        printf("createShaderModuleFromBinary succeeded.");

        free(pShaderCode);
        return shaderModule;

    }

}