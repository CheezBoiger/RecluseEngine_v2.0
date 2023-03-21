//
#include "VulkanShaderCache.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Types.hpp"

#include <unordered_map>

namespace Recluse {

namespace ShaderPrograms {

// Cache holds all system cache info.
std::unordered_map<ShaderId, std::unordered_map<ShaderPermutationId, VkShaderModule>>                       shaderCache;
::std::unordered_map<ShaderProgramId, std::unordered_map<ShaderProgramPermutation, VulkanShaderProgram>>    cache;

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

    if (shaderCache.find(pShader->getHashId()) != shaderCache.end())
    {
        if (shaderCache[pShader->getHashId()].find(pShader->getPermutationId()) != shaderCache[pShader->getHashId()].end())
        {
            *pModule = shaderCache[pShader->getHashId()][pShader->getPermutationId()];
            return VK_SUCCESS;
        }
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

    if (pfn_vkSetDebugUtilsObjectNameEXT)
    {
        std::string strName = pShader->getName();
        strName += std::to_string(pShader->getPermutationId());

        VkDebugUtilsObjectNameInfoEXT nameInfo  = { };
        nameInfo.sType                          = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType                     = VK_OBJECT_TYPE_SHADER_MODULE;
        nameInfo.pNext                          = nullptr;
        nameInfo.pObjectName                    = strName.c_str();
        nameInfo.objectHandle                   = reinterpret_cast<uint64_t>(shaderModule);
        result = pfn_vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
        R_ASSERT(result == VK_SUCCESS);
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
        R_ASSERT_MSG(definition.graphics.vs, "Must be able to have at least a vertex shader in order to build shader program for Vulkan!");
        createShaderModule(device, definition.graphics.vs, &programOut.graphics.vs);
        createShaderModule(device, definition.graphics.ps, &programOut.graphics.ps);
        createShaderModule(device, definition.graphics.gs, &programOut.graphics.gs);
        createShaderModule(device, definition.graphics.hs, &programOut.graphics.hs);
        createShaderModule(device, definition.graphics.ds, &programOut.graphics.ds);
        programOut.graphics.vsEntry = definition.graphics.vs->getEntryPointName();
        programOut.graphics.psEntry = definition.graphics.ps ? definition.graphics.ps->getEntryPointName() : nullptr;
        programOut.graphics.gsEntry = definition.graphics.gs ? definition.graphics.gs->getEntryPointName() : nullptr;
        programOut.graphics.dsEntry = definition.graphics.ds ? definition.graphics.ds->getEntryPointName() : nullptr;
        programOut.graphics.hsEntry = definition.graphics.hs ? definition.graphics.hs->getEntryPointName() : nullptr;
        break;
    case BindType_Compute: 
        programOut.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE; 
        R_ASSERT_MSG(definition.compute.cs, "Must have at least a compute shader in order to build shader program for Vulkan!");
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
        programOut.raytrace.rayAnyHitEntry = definition.raytrace.rany ? definition.raytrace.rany->getEntryPointName() : nullptr;
        programOut.raytrace.rayClosestEntry = definition.raytrace.rclosest ? definition.raytrace.rclosest->getEntryPointName() : nullptr;
        programOut.raytrace.rayGenEntry = definition.raytrace.rgen ? definition.raytrace.rgen->getEntryPointName() : nullptr;
        programOut.raytrace.rayIntersectEntry = definition.raytrace.rintersect ? definition.raytrace.rintersect->getEntryPointName() : nullptr;
        programOut.raytrace.rayMissEntry = definition.raytrace.rmiss ? definition.raytrace.rmiss->getEntryPointName() : nullptr;
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

ErrType createDebugShaderNames(VulkanDevice* pDevice, const VulkanShaderProgram& program)
{
    Bool supportsDebugMarking = pDevice->getAdapter()->getInstance()->supportsDebugMarking();
    if (supportsDebugMarking)
    {
        ;
    }
    return RecluseResult_Ok;
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