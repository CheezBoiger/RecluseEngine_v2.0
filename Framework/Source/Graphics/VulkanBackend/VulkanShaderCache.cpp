//
#include "VulkanShaderCache.hpp"
#include "VulkanDevice.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Types.hpp"

#include <unordered_map>

namespace Recluse {

namespace ShaderPrograms {

// Cache holds all system cache info.
::std::unordered_map<ShaderProgramId, std::unordered_map<ShaderProgramPermutation, VulkanShaderProgram>> cache;

static VkResult createShaderModule(VkDevice device, Shader* pShader, VkShaderModule* pModule)
{
    if (!pShader)
    {
        *pModule = VK_NULL_HANDLE;
        return VK_SUCCESS;
    }

    if (!pModule || !device)
    {
        R_ERR(__FUNCTION__, "Either module or device were passed as NULL!! Can not create a VkShaderModule!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkShaderModule shaderModule     = VK_NULL_HANDLE;
    VkShaderModuleCreateInfo info   = { };

    info.sType                      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pCode                      = (U32*)pShader->getByteCode();
    info.codeSize                   = (SizeT)pShader->getSzBytes();
    
    VkResult result                 = vkCreateShaderModule(device, &info, nullptr, &shaderModule);

    if (result != VK_SUCCESS)
    {
        R_ERR("VulkanShaderCache", "Failed (result = %d)", result);
    }
    else
    {
        *pModule = shaderModule;
    }

    return result;
}


VulkanShaderProgram createShaderProgram(VkDevice device, const Builder::ShaderProgramDefinition& definition)
{
    VulkanShaderProgram programOut = { };

    switch (definition.pipelineType)
    {
    case BindType_Graphics: 
        programOut.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        createShaderModule(device, definition.graphics.vs, &programOut.graphics.vs);
        createShaderModule(device, definition.graphics.ps, &programOut.graphics.ps);
        createShaderModule(device, definition.graphics.gs, &programOut.graphics.gs);
        createShaderModule(device, definition.graphics.hs, &programOut.graphics.hs);
        createShaderModule(device, definition.graphics.ds, &programOut.graphics.ds);
        programOut.graphics.vsEntry = definition.graphics.vs->getEntryPointName();
        programOut.graphics.psEntry = definition.graphics.ps->getEntryPointName();
        programOut.graphics.gsEntry = definition.graphics.gs->getEntryPointName();
        programOut.graphics.dsEntry = definition.graphics.ds->getEntryPointName();
        programOut.graphics.hsEntry = definition.graphics.hs->getEntryPointName();
        break;
    case BindType_Compute: 
        programOut.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE; 
        createShaderModule(device, definition.compute.cs, &programOut.compute.cs);
        programOut.compute.csEntry = definition.compute.cs->getEntryPointName();
        break;
    case BindType_RayTrace: 
        programOut.bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR; 
        createShaderModule(device, definition.raytrace.rany, &programOut.raytrace.rayAnyHit);
        createShaderModule(device, definition.raytrace.rclosest, &programOut.raytrace.rayClosest);
        createShaderModule(device, definition.raytrace.rgen, &programOut.raytrace.rayGen);
        createShaderModule(device, definition.raytrace.rintersect, &programOut.raytrace.rayIntersect);
        createShaderModule(device, definition.raytrace.rmiss, &programOut.raytrace.rayMiss);
        programOut.raytrace.rayAnyHitEntry = definition.raytrace.rany->getEntryPointName();
        programOut.raytrace.rayClosestEntry = definition.raytrace.rclosest->getEntryPointName();
        programOut.raytrace.rayGenEntry = definition.raytrace.rgen->getEntryPointName();
        programOut.raytrace.rayIntersectEntry = definition.raytrace.rintersect->getEntryPointName();
        programOut.raytrace.rayMissEntry = definition.raytrace.rmiss->getEntryPointName();
        break;
    }

    return programOut;
}

void destroyShaderModule(VulkanDevice* pDevice, VkShaderModule m)
{
    R_ASSERT(m);
    if (m)
    {
        return;
    }
    vkDestroyShaderModule(pDevice->get(), m, nullptr);
}

VulkanShaderProgram* obtainShaderProgram(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation)
{
    
    if (isProgramCached(shaderProgram, permutation)) 
    {
        return &cache[shaderProgram][permutation];
    }
    return nullptr;
}


ErrType loadNativeShaderProgramPermutation(VulkanDevice* pDevice, ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition)
{
    auto& it = cache[shaderProgram].find(permutation);
    if (it != cache[shaderProgram].end())
    {
        return RecluseResult_NeedsUpdate;
    }

    VulkanShaderProgram program = createShaderProgram(pDevice->get(), definition);
    cache[shaderProgram][permutation] = program;

    return RecluseResult_Ok;
}


B32 isProgramCached(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation)
{
    auto it = cache.find(shaderProgram);
    if (it != cache.end())
    {
        return it->second.find(permutation) != it->second.end();
    }
    return false;
}


B32 isProgramCached(ShaderProgramId shaderProgram)
{
    return (cache.find(shaderProgram) != cache.end());
}


static void destroyShaderProgramModules(VkDevice device, VulkanShaderProgram& program)
{
    switch (program.bindPoint)
    {
    case VK_PIPELINE_BIND_POINT_GRAPHICS:
        vkDestroyShaderModule(device, program.graphics.vs, nullptr);
        vkDestroyShaderModule(device, program.graphics.ps, nullptr);
        vkDestroyShaderModule(device, program.graphics.gs, nullptr);
        vkDestroyShaderModule(device, program.graphics.hs, nullptr);
        vkDestroyShaderModule(device, program.graphics.ds, nullptr);
        break;
    case VK_PIPELINE_BIND_POINT_COMPUTE:
        vkDestroyShaderModule(device, program.compute.cs, nullptr);
        break;
    case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
        vkDestroyShaderModule(device, program.raytrace.rayAnyHit, nullptr);
        vkDestroyShaderModule(device, program.raytrace.rayClosest, nullptr);
        vkDestroyShaderModule(device, program.raytrace.rayGen, nullptr);
        vkDestroyShaderModule(device, program.raytrace.rayIntersect, nullptr);
        vkDestroyShaderModule(device, program.raytrace.rayMiss, nullptr);
    }
}


void unloadAll(VulkanDevice* pDevice)
{
    VkDevice device = pDevice->get();
    for (auto keyval : cache) 
    {
        for (auto permutation : keyval.second) 
        {
            auto& perm = permutation.second;
            destroyShaderProgramModules(device, perm);
        }

        keyval.second.clear();
    }

    cache.clear();
}


ErrType unloadProgram(VulkanDevice* pDevice, ShaderProgramId program)
{
    auto it = cache.find(program);
    if (it == cache.end())
    {
        return RecluseResult_NotFound;
    }

    for (auto& permutation : it->second)
    { 
        destroyShaderProgramModules(pDevice->get(), permutation.second);
    }
    cache.erase(it);
    return RecluseResult_Ok;
}
} // ShaderPrograms
} // Recluse