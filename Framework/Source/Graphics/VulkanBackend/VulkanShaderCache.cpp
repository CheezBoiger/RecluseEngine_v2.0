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
        R_ERROR(__FUNCTION__, "Either module or device were passed as NULL!! Can not create a VkShaderModule!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (shaderCache.find(pShader->getShaderHashId()) != shaderCache.end())
    {
        if (shaderCache[pShader->getShaderHashId()].find(pShader->getPermutationId()) != shaderCache[pShader->getShaderHashId()].end())
        {
            *pModule = shaderCache[pShader->getShaderHashId()][pShader->getPermutationId()];
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
        R_ERROR("VulkanShaderCache", "Failed (result = %d)", result);
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


void copyName(char* dst, Shader* pShader)
{
    if (pShader)
    {
        size_t lenBytes = strlen(pShader->getEntryPointName());
        R_ASSERT_FORMAT(lenBytes < 36, "Entry point size must be less than 36 characters! EntryName=%s", pShader->getEntryPointName());
        memcpy(dst, pShader->getEntryPointName(), lenBytes);
    }
}


VulkanShaderProgram createShaderProgram(VkDevice device, const ShaderProgramDefinition& definition)
{
    VulkanShaderProgram programOut = { };
    memset(&programOut, 0, sizeof(VulkanShaderProgram));
    switch (definition.pipelineType)
    {
    case BindType_Graphics: 
        programOut.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        programOut.graphics.usesMeshShaders = definition.graphics.usesMeshShaders;
        if (programOut.graphics.usesMeshShaders)
        {
            R_ASSERT_FORMAT(definition.graphics.as && definition.graphics.ms, "Both Task and Mesh shaders must be available when building this shader program for Vulkan!");
            createShaderModule(device, definition.graphics.as, &programOut.graphics.as);
            createShaderModule(device, definition.graphics.ms, &programOut.graphics.ms);
            copyName(programOut.graphics.asEntry, definition.graphics.as);
            copyName(programOut.graphics.msEntry, definition.graphics.ms);
        }
        else
        {
            R_ASSERT_FORMAT(definition.graphics.vs, "Must be able to have at least a vertex shader in order to build shader program for Vulkan!");
            createShaderModule(device, definition.graphics.vs, &programOut.graphics.vs);
            createShaderModule(device, definition.graphics.gs, &programOut.graphics.gs);
            createShaderModule(device, definition.graphics.hs, &programOut.graphics.hs);
            createShaderModule(device, definition.graphics.ds, &programOut.graphics.ds);
            copyName(programOut.graphics.vsEntry, definition.graphics.vs);
            copyName(programOut.graphics.gsEntry, definition.graphics.gs);
            copyName(programOut.graphics.dsEntry, definition.graphics.ds);
            copyName(programOut.graphics.hsEntry, definition.graphics.hs);
        }
        createShaderModule(device, definition.graphics.ps, &programOut.graphics.ps);
        copyName(programOut.graphics.psEntry, definition.graphics.ps);
        break;
    case BindType_Compute: 
        programOut.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE; 
        R_ASSERT_FORMAT(definition.compute.cs, "Must have at least a compute shader in order to build shader program for Vulkan!");
        createShaderModule(device, definition.compute.cs, &programOut.compute.cs);
        copyName(programOut.compute.csEntry, definition.compute.cs);
        break;
    case BindType_RayTrace: 
        programOut.bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR; 
        createShaderModule(device, definition.raytrace.rany, &programOut.raytrace.rayAnyHit);
        createShaderModule(device, definition.raytrace.rclosest, &programOut.raytrace.rayClosest);
        createShaderModule(device, definition.raytrace.rgen, &programOut.raytrace.rayGen);
        createShaderModule(device, definition.raytrace.rintersect, &programOut.raytrace.rayIntersect);
        createShaderModule(device, definition.raytrace.rmiss, &programOut.raytrace.rayMiss);
        copyName(programOut.raytrace.rayAnyHitEntry, definition.raytrace.rany);
        copyName(programOut.raytrace.rayClosestEntry, definition.raytrace.rclosest);
        copyName(programOut.raytrace.rayGenEntry, definition.raytrace.rgen);
        copyName(programOut.raytrace.rayIntersectEntry, definition.raytrace.rintersect);
        copyName(programOut.raytrace.rayMissEntry, definition.raytrace.rmiss);
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

ResultCode createDebugShaderNames(VulkanDevice* pDevice, const VulkanShaderProgram& program)
{
    Bool supportsDebugMarking = pDevice->getAdapter()->getInstance()->supportsDebugMarking();
    if (supportsDebugMarking)
    {
        ;
    }
    return RecluseResult_Ok;
}


ResultCode loadNativeShaderProgramPermutation(VulkanDevice* pDevice, ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition)
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
        if (program.graphics.usesMeshShaders)
        {
            vkDestroyShaderModule(device, program.graphics.as, nullptr);
            vkDestroyShaderModule(device, program.graphics.ms, nullptr);
        }
        else
        {
            vkDestroyShaderModule(device, program.graphics.vs, nullptr);
            vkDestroyShaderModule(device, program.graphics.gs, nullptr);
            vkDestroyShaderModule(device, program.graphics.hs, nullptr);
            vkDestroyShaderModule(device, program.graphics.ds, nullptr);
        }
        vkDestroyShaderModule(device, program.graphics.ps, nullptr);
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


ResultCode unloadProgram(VulkanDevice* pDevice, ShaderProgramId program)
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