//
#include "Recluse/Pipeline/Graphics/Reflection/SpirvReflection.hpp"
#include "Recluse/Math/MathCommons.hpp"

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
            std::vector<ShaderBind> cbvBindings;
            std::vector<ShaderBind> srvsBindings;
            std::vector<ShaderBind> uavBindings;
            std::vector<ShaderBind> samplerBindings;
            uint cbvOffset = 0xff;
            uint srvOffset = 0xff;
            uint uavOffset = 0xff;
            uint samplerOffset = 0xff;
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
                            //reflectionOutput.samplers.push_back(shaderBind);
                            samplerBindings.push_back(shaderBind);
                            samplerOffset = Math::minimum((uint)dstBinding, samplerOffset);
                            metadata.numSamplers += 1;
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
                            //reflectionOutput.uavs.push_back(shaderBind);
                            uavBindings.push_back(dstBinding);
                            uavOffset = Math::minimum((uint)dstBinding, (uint)uavOffset);
                            metadata.numUavs += 1;
                        }
                        if (descriptorBind->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SRV)
                        {
                            //reflectionOutput.srvs.push_back(shaderBind);
                            srvsBindings.push_back(shaderBind);
                            srvOffset = Math::minimum((uint)dstBinding, (uint)srvOffset);
                            metadata.numSrvs += 1;
                        }
                        break;
                    }
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    {
                        if (descriptorBind->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SRV)
                        {
                            //reflectionOutput.srvs.push_back(shaderBind);
                            srvsBindings.push_back(shaderBind);
                            srvOffset = Math::minimum((uint)dstBinding, (uint)srvOffset);
                            metadata.numSrvs += 1;
                        }
                        break;
                    }
                    case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    {
                        if (descriptorBind->resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_CBV)
                        {
                            //reflectionOutput.cbvs.push_back(shaderBind);
                            cbvBindings.push_back(shaderBind);
                            cbvOffset = Math::minimum((uint)dstBinding, (uint)cbvOffset);
                            metadata.numCbvs += 1;
                        }
                        break;
                    }
                }
            }
            
            // use the base factors as the offset.
            ShaderReflectionInformation::Metadata& metadata = reflectionOutput.perSetMetadata[descriptorSetIdx];
            metadata.baseCbv = cbvOffset;
            metadata.baseSrv = metadata.numSrvs != 0 ? srvOffset : 0;
            metadata.baseUav = metadata.numUavs != 0 ? uavOffset : 0;
            metadata.baseSampler = metadata.numSamplers ? samplerOffset : 0;
            
            // Last pass is to add all bindings as a virtual offset.
            for (U32 binding : cbvBindings)
            {
                U16 registerIdx = binding - metadata.baseCbv;
                U16 space = ShaderReflectionInformation::unpackShaderSet(binding);
                ShaderBind bind = ShaderReflectionInformation::packShaderBinding(space, registerIdx);
                reflectionOutput.cbvs.push_back(bind);
            }

            for (U32 binding : srvsBindings)
            {
                U16 registerIdx = binding - metadata.baseSrv;
                U16 space = ShaderReflectionInformation::unpackShaderSet(binding);
                ShaderBind bind = ShaderReflectionInformation::packShaderBinding(space, registerIdx);
                reflectionOutput.srvs.push_back(bind);
            }

            for (U32 binding : uavBindings)
            {
                U16 registerIdx = binding - metadata.baseUav;
                U16 space = ShaderReflectionInformation::unpackShaderSet(binding);
                ShaderBind bind = ShaderReflectionInformation::packShaderBinding(space, registerIdx);
                reflectionOutput.uavs.push_back(bind);
            }

            for (U32 binding : samplerBindings)
            {
                U16 registerIdx = binding - metadata.baseSampler;
                U16 space = ShaderReflectionInformation::unpackShaderSet(binding);
                ShaderBind bind = ShaderReflectionInformation::packShaderBinding(space, registerIdx);
                reflectionOutput.samplers.push_back(bind);
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