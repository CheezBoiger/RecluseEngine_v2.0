//
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"

#include "Recluse/Messaging.hpp"

#if defined RCL_GLSLANG

#define ENABLE_HLSL 1
#include <vulkan/vulkan_core.h>
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/Types.h"
#include "glslang/SPIRV/GlslangToSpv.h" 
#include "glslang/Include/ResourceLimits.h"

#include "SPIRV-Reflect/spirv_reflect.h"

const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
#if !R_GLSLANG_LEGACY_API
    /* .maxMeshOutputVerticesEXT */ 256,
    /* .maxMeshOutputPrimitivesEXT */ 512,
    /* .maxMeshWorkGroupSizeX_EXT */ 32,
    /* .maxMeshWorkGroupSizeY_EXT */ 1,
    /* .maxMeshWorkGroupSizeZ_EXT */ 1,
    /* .maxTaskWorkGroupSizeX_EXT */ 32,
    /* .maxTaskWorkGroupSizeY_EXT */ 1,
    /* .maxTaskWorkGroupSizeZ_EXT */ 1,
    /* .maxMeshViewCountEXT */  4,
    /* .maxDualSourceDrawBuffersEXT */ 1,
#endif
    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};
#endif

namespace Recluse {
namespace Pipeline {

#if defined RCL_GLSLANG

class GlslangShaderBuilder : public ShaderBuilder 
{
public:
    GlslangShaderBuilder(ShaderIntermediateCode imm)
        : ShaderBuilder(imm)
        , m_isInitialized(false) { }

    ResultCode setUp() override 
    {
        glslang::InitializeProcess();
        m_isInitialized = true;
        return RecluseResult_Ok;
    }

    ResultCode tearDown() override
    {
        R_ASSERT(m_isInitialized == true);
        glslang::FinalizeProcess();
        m_isInitialized = false;
        return RecluseResult_Ok;
    }

    ResultCode onCompile
        (
            const std::vector<char>& srcCode, 
            std::vector<char>& byteCode, 
            const char* entryPoint,
            ShaderLanguage lang, 
            ShaderType shaderType, 
            const std::vector<PreprocessDefine>& defines = std::vector<PreprocessDefine>()
        ) override
    {
        R_DEBUG("GLSLANG", "Compiling shader...");
        R_ASSERT(m_isInitialized == true);

        EShLanguage stage       = EShLangVertex;
        std::vector<std::vector<char>> nativeDefines;
        std::vector<const char*> strings;

        if (!defines.empty())
        {
            strings.resize(defines.size() + 1);
            nativeDefines.resize(defines.size());
            std::string defineDirective = "#define";
            for (U32 i = 0; i < nativeDefines.size(); ++i)
            {
                //+9 for directive +1 for space, +2 for \n\0
                nativeDefines[i].resize(defineDirective.size() + 1 + defines[i].variable.size() + 1 + defines[i].value.size() + 2);
                std::string merge = defineDirective + " " + defines[i].variable + " " + defines[i].value + "\n\0";
                memcpy(nativeDefines[i].data(), merge.data(), merge.size());
                strings[i] = nativeDefines[i].data();
            }
        }
        else
        {
            strings.resize(1);
        }

        strings.back() = srcCode.data();

        switch (shaderType) 
        {
            case ShaderType_Vertex:                 stage = EShLangVertex; break;
            case ShaderType_Fragment:               stage = EShLangFragment; break;
            case ShaderType_Compute:                stage = EShLangCompute; break;
            case ShaderType_TessellationControl:    stage = EShLangTessControl; break;
            case ShaderType_TessellationEvaluation: stage = EShLangTessEvaluation; break;
            // TODO: Support more shaders!
        }

        glslang::TProgram program;
        glslang::TShader shader(stage);
        std::vector<U32> spirv;
        spv::SpvBuildLogger logger;
        std::string compileLog;
        glslang::EShClient client           = glslang::EShClientVulkan;
        I32 clientVersion                   = 100;
        EShMessages messages                = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);

        if (lang == ShaderLanguage_Hlsl) 
        {
            R_DEBUG("GLSLANG", "HLSL used, compiling to SPIRV...");

            shader.setEnvInput(glslang::EShSourceHlsl, stage, client, clientVersion);            
            shader.setHlslIoMapping(true);
            shader.setAutoMapBindings(true);
            messages = (EShMessages)((I32)messages | (I32)EShMsgReadHlsl);
        }
        else
        {
            shader.setAutoMapLocations(true);   // --aml, which will allow for implicit location mapping 
                                                // for builtin blocks.
        }

        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        shader.setStrings(strings.data(), strings.size());
        shader.setEntryPoint(entryPoint);
    
        if (!shader.parse(&DefaultTBuiltInResource, 450, false, messages)) 
        {
            R_ERROR("GLSLANG", "Compile Error: %s", shader.getInfoLog());

            return RecluseResult_Failed;
        }

        program.addShader(&shader);
    
        if (!program.link(messages)) 
        {
            R_ERROR("GLSLANG", "Failed to link shader program!");

            return RecluseResult_Failed;
        }

        glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger);
        byteCode.resize(spirv.size() * sizeof(U32)); // Byte code is in uint32 format.
        memcpy(byteCode.data(), spirv.data(), byteCode.size());
        return RecluseResult_Ok;
    }

    ResultCode reflect(ShaderReflection& reflectionOutput, const char* bytecode, U64 sizeBytes, ShaderLanguage lang) override
    {
        SpvReflectShaderModule reflectModule;
        SpvReflectResult result = spvReflectCreateShaderModule(sizeBytes, bytecode, &reflectModule);
        ResultCode outResult = RecluseResult_Ok;
        if (result == SPV_REFLECT_RESULT_SUCCESS)
        {
            std::vector<SpvReflectInterfaceVariable*> inputVars;
            uint32_t inputVarCount = 0;
            result = spvReflectEnumerateInputVariables(&reflectModule, &inputVarCount, nullptr);
            R_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);
            inputVars.resize(inputVarCount);
            result = spvReflectEnumerateInputVariables(&reflectModule, &inputVarCount, inputVars.data());
            R_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);
            for (U32 i = 0; i < inputVarCount; ++i)
            {
                SpvReflectInterfaceVariable* variable = inputVars[i];
                variable;
            }
            uint32_t descriptorSetCount = 0;
            spvReflectEnumerateDescriptorSets(&reflectModule, &descriptorSetCount, nullptr);
            std::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
            spvReflectEnumerateDescriptorSets(&reflectModule, &descriptorSetCount, descriptorSets.data());
            for (U32 descriptorSetIdx = 0; descriptorSetIdx < descriptorSetCount; ++descriptorSetIdx)
            {
                SpvReflectDescriptorSet* set = descriptorSets[descriptorSetIdx];
                uint32_t bindingCount = set->binding_count;
                for (uint32_t bindingIdx = 0; bindingIdx < bindingCount; ++bindingIdx)
                {
                    // Binds for GLSL is based on binding locations. This can vary, and is not in a table, so 
                    // it makes it a little more of an effort to piece the inputs together.
                    SpvReflectDescriptorBinding* binding = set->bindings[bindingIdx];
                    switch (binding->descriptor_type)
                    {
                        case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                        {
                            if (binding->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SAMPLER)
                            {
                                reflectionOutput.metadata.numSamplers += 1;
                                reflectionOutput.samplers.push_back(static_cast<ShaderBind>(binding->binding));
                            }
                            break;
                        }
                        case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        {
                            // Storage buffers or images, they can either be treated as SRVs, or UAVs depending on 
                            // their access in shader code.
                            if (binding->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_UAV)
                            {
                                reflectionOutput.metadata.numUavs += 1;
                                reflectionOutput.uavs.push_back(static_cast<ShaderBind>(binding->binding));
                            }
                            if (binding->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SRV)
                            {
                                reflectionOutput.metadata.numSrvs += 1;
                                reflectionOutput.srvs.push_back(static_cast<ShaderBind>(binding->binding));
                            }
                            break;
                        }
                        case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        {
                            if (binding->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SRV)
                            {
                                reflectionOutput.metadata.numSrvs += 1;
                                reflectionOutput.srvs.push_back(static_cast<ShaderBind>(binding->binding));
                            }
                            break;
                        }
                        case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        {
                            if (binding->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_CBV)
                            {
                                reflectionOutput.metadata.numCbvs += 1;
                                reflectionOutput.cbvs.push_back(static_cast<ShaderBind>(binding->binding));
                            }
                            break;
                        }
                    }
                    
                }
            }
            spvReflectDestroyShaderModule(&reflectModule);
            outResult = RecluseResult_Ok;
        }
        else
        {
            outResult = RecluseResult_Failed;
        }
        return outResult;
    }

private:
    Bool m_isInitialized;
};

#endif

ShaderBuilder* createGlslangShaderBuilder(ShaderIntermediateCode imm)
{
#if defined RCL_GLSLANG
    return new GlslangShaderBuilder(imm);
#else
    R_ERR("GLSLANG", "Glslang not enabled for compilation!");
    return nullptr;
#endif
}
} // Pipeline
} // Recluse