//
#include "Recluse/Pipeline/Graphics/Reflection/SpirvReflection.hpp"

#include "Recluse/Messaging.hpp"
#include "SPIRV-Reflect/spirv_reflect.h"

namespace Recluse {
namespace Pipeline {


ResultCode SpirvReflection::reflect(ShaderReflectionInformation& reflectionOutput, const Shader* shader)
{
    const char* bytecode = shader->getByteCode();
    U64 sizeBytes = shader->getSzBytes();

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
                SpvReflectDescriptorBinding* descriptorBind = set->bindings[bindingIdx];
                const U16 dstBinding = descriptorBind->binding;
                const U16 dstSet = descriptorBind->set;

                if (dstSet >= reflectionOutput.perSetMetadata.size())
                    reflectionOutput.perSetMetadata.resize(dstSet + 1);
                ShaderReflectionInformation::Metadata& metadata = reflectionOutput.perSetMetadata[dstSet];

                // We want to ensure they fit in an unsigned short.
                R_ASSERT_FORMAT(dstBinding < 65535 && dstSet < 65535, "The shader destination set or binding exceeds the expected registers to be properly reflected!");

                // TODO(): This packing is specific to SPIRV GLSL, so we will need to have some way to properly make this universal to the framework, and
                // Vulkan.
                const ShaderBind shaderBind = ShaderReflectionInformation::packShaderBinding(dstSet, dstBinding);

                switch (descriptorBind->descriptor_type)
                {
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                    {
                        if (descriptorBind->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SAMPLER)
                        {
                            metadata.numSamplers += 1;
                            reflectionOutput.samplers.push_back(shaderBind);
                        }
                        break;
                    }
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    {
                        // Storage buffers or images, they can either be treated as SRVs, or UAVs depending on 
                        // their access in shader code.
                        if (descriptorBind->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_UAV)
                        {
                            metadata.numUavs += 1;
                            reflectionOutput.uavs.push_back(shaderBind);
                        }
                        if (descriptorBind->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SRV)
                        {
                            metadata.numSrvs += 1;
                            reflectionOutput.srvs.push_back(shaderBind);
                        }
                        break;
                    }
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    {
                        if (descriptorBind->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SRV)
                        {
                            metadata.numSrvs += 1;
                            reflectionOutput.srvs.push_back(shaderBind);
                        }
                        break;
                    }
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    {
                        if (descriptorBind->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_CBV)
                        {
                            metadata.numCbvs += 1;
                            reflectionOutput.cbvs.push_back(shaderBind);
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
} // Pipeline
} // Recluse