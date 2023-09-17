//
#pragma once

#include "Recluse/Types.hpp"
#include "VulkanCommons.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"
#include <unordered_map>

namespace Recluse {

class Shader;
class VulkanDevice;

namespace ShaderPrograms {
    struct VulkanShaderProgram
    {
        VkPipelineBindPoint bindPoint;
        union 
        {
            struct 
            {
                union
                {
                    struct
                    {
                        VkShaderModule as;
                        char asEntry[36];
                        VkShaderModule ms;
                        char msEntry[36];
                    };
                    struct
                    {
                        VkShaderModule vs;
                        char vsEntry[36];
                        VkShaderModule gs;
                        char gsEntry[36];
                        VkShaderModule hs;
                        char hsEntry[36];
                        VkShaderModule ds;
                        char dsEntry[36];
                    };
                };
                VkShaderModule ps;
                char psEntry[36];
                Bool usesMeshShaders;
            } graphics;
            struct
            {
                VkShaderModule cs;
                char csEntry[36];
            } compute;
            struct
            {
                VkShaderModule rayGen;
                char rayGenEntry[36];
                VkShaderModule rayMiss;
                char rayMissEntry[36];
                VkShaderModule rayIntersect;
                char rayIntersectEntry[36];
                VkShaderModule rayAnyHit;
                char rayAnyHitEntry[36];
                VkShaderModule rayClosest;
                char rayClosestEntry[36];
            } raytrace;
        };
    };

    // Get the shader cache module. This is the native vulkan module.
    VulkanShaderProgram* obtainShaderProgram(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);

    // Cache the shader, this also checks if a VkShaderModule is also present. If not,
    // Will create one.
    ResultCode          loadNativeShaderProgramPermutation(VulkanDevice* pDevice, ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition);

    // Check if the given shader has already been cached.
    B32                 isProgramCached(ShaderProgramId shaderProgram);
    B32                 isProgramCached(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);

    // Clear the whole cache from this shader. Should be called when app closes!
    void                unloadAll(VulkanDevice* pDevice);
    ResultCode          unloadProgram(VulkanDevice* pDevice, ShaderProgramId program);
} // ShaderPrograms
} // Recluse