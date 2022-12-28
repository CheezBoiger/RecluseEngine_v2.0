//
#pragma once

#include "Recluse/Types.hpp"
#include "VulkanCommons.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Recluse/Graphics/ShaderProgramBuilder.hpp"
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
                VkShaderModule vs;
                const char* vsEntry;
                VkShaderModule ps;
                const char* psEntry;
                VkShaderModule gs;
                const char* gsEntry;
                VkShaderModule hs;
                const char* hsEntry;
                VkShaderModule ds;
                const char* dsEntry;
            } graphics;
            struct
            {
                VkShaderModule cs;
                const char* csEntry;
            } compute;
            struct
            {
                VkShaderModule rayGen;
                const char* rayGenEntry;
                VkShaderModule rayMiss;
                const char* rayMissEntry;
                VkShaderModule rayIntersect;
                const char* rayIntersectEntry;
                VkShaderModule rayAnyHit;
                const char* rayAnyHitEntry;
                VkShaderModule rayClosest;
                const char* rayClosestEntry;
            } raytrace;
        };
    };

    // Get the shader cache module. This is the native vulkan module.
    VulkanShaderProgram* obtainShaderProgram(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);

    // Cache the shader, this also checks if a VkShaderModule is also present. If not,
    // Will create one.
    ErrType         loadNativeShaderProgramPermutation(VulkanDevice* pDevice, ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition);

    // Check if the given shader has already been cached.
    B32             isProgramCached(ShaderProgramId shaderProgram);
    B32             isProgramCached(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);

    // Clear the whole cache from this shader. Should be called when app closes!
    void            unloadAll(VulkanDevice* pDevice);

    ErrType unloadProgram(VulkanDevice* pDevice, ShaderProgramId program);
} // ShaderPrograms
} // Recluse