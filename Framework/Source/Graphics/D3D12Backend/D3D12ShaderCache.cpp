//
#include "Recluse/Messaging.hpp"
#include "D3D12ShaderCache.hpp"

#include <unordered_map>
#include <map>

#include <d3dcompiler.h>

namespace Recluse {
namespace D3D {
namespace Cache {


std::unordered_map<ShaderProgramId, std::unordered_map<ShaderProgramPermutation, D3DShaderProgram>> g_shaderProgramMap;
std::map<Hash64, std::map<ShaderProgramPermutation, ID3DBlob*>> g_cachedShaderBlobs;


R_INTERNAL
ID3DBlob* createBlob(SizeT sizeBytes)
{
    ID3DBlob* pBlob = nullptr;
    HRESULT result = D3DCreateBlob(sizeBytes, &pBlob);
    R_ASSERT(SUCCEEDED(result));
    return pBlob;
}



ID3DBlob* makeBlob(Shader* pShader)
{
    ID3DBlob* pOutput = nullptr;
    if (pShader)
    {
        Hash64 shaderHash = pShader->getShaderHashId();
        ShaderPermutationId permutation = pShader->getPermutationId();
        auto shaderIter = g_cachedShaderBlobs.find(shaderHash);
        if (shaderIter != g_cachedShaderBlobs.end())
        {
            auto permIter = shaderIter->second.find(permutation);
            if (permIter != shaderIter->second.end())
            {
                permIter->second->AddRef();
                pOutput = permIter->second;
            }
        }
        else
        {
            ID3DBlob* pBlob;
            HRESULT hr = D3DCreateBlob(pShader->getSzBytes(), &pBlob);
            R_ASSERT(SUCCEEDED(hr));
            memcpy(pBlob->GetBufferPointer(), pShader->getByteCode(), pShader->getSzBytes());
            g_cachedShaderBlobs[shaderHash][permutation] = pBlob;
            pOutput = pBlob;
        }
    }
    return pOutput;
}


R_INTERNAL
void callReleaseBlob(ID3DBlob* pBlob)
{
    if (pBlob)
    {
        ULONG ref = pBlob->Release();
        if (ref == 0)
        {
            // TODO: Need to hold onto the permutation and shader hash info to find the cached shader blob!
        }
    }
}


R_INTERNAL 
Bool destroyShaderProgram(D3DShaderProgram* pProgram)
{
    if (!pProgram)
    {
        return false;
    }
    switch (pProgram->bindType)
    {
        case BindType_Graphics:
        {
            callReleaseBlob(pProgram->graphics.psBytecode);
            callReleaseBlob(pProgram->graphics.vsBytecode);
            callReleaseBlob(pProgram->graphics.hsBytecode);
            callReleaseBlob(pProgram->graphics.dsBytecode);
            callReleaseBlob(pProgram->graphics.gsBytecode);
            break;
        }
        case BindType_Compute:
        {
            callReleaseBlob(pProgram->compute.csBytecode);
            break;
        }
        case BindType_RayTrace:
        {
            callReleaseBlob(pProgram->raytrace.rayAny);
            callReleaseBlob(pProgram->raytrace.rayClosest);
            callReleaseBlob(pProgram->raytrace.rayGen);
            callReleaseBlob(pProgram->raytrace.rayIntersect);
            callReleaseBlob(pProgram->raytrace.rayMiss);
            break;
        }
        default:
        {
            R_ERROR(R_CHANNEL_D3D12, "D3DShaderProgram does not have a proper bind type! Unable to resolve destruction");
        }
    }
    return true;
}


R_INTERNAL
Bool internalMakeShaderProgram(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition)
{
    D3DShaderProgram program    = { };
    program.bindType            = definition.pipelineType;
    switch (definition.pipelineType)
    {
        case BindType_Graphics:
        {
            program.graphics.vsBytecode = makeBlob(definition.graphics.vs);
            program.graphics.vsMainEntry = definition.graphics.vs ? definition.graphics.vs->getEntryPointName() : nullptr;
            program.graphics.dsBytecode = makeBlob(definition.graphics.ds);
            program.graphics.dsMainEntry = definition.graphics.ds ? definition.graphics.ds->getEntryPointName() : nullptr;
            program.graphics.dsBytecode = makeBlob(definition.graphics.ps);
            program.graphics.dsMainEntry = definition.graphics.ps ? definition.graphics.ps->getEntryPointName() : nullptr;
            program.graphics.gsBytecode = makeBlob(definition.graphics.gs);
            program.graphics.gsMainEntry = definition.graphics.gs ? definition.graphics.gs->getEntryPointName() : nullptr;
            program.graphics.hsBytecode = makeBlob(definition.graphics.hs);
            program.graphics.hsMainEntry = definition.graphics.hs ? definition.graphics.hs->getEntryPointName() : nullptr;
            break;
        }
        case BindType_Compute:
        {
            program.compute.csBytecode = makeBlob(definition.compute.cs);
            program.compute.csMainEntry = definition.compute.cs ? definition.compute.cs->getEntryPointName() : nullptr;
            break;
        }
        case BindType_RayTrace:
        {
            program.raytrace.rayAny = makeBlob(definition.raytrace.rany);
            program.raytrace.rayAnyMainEntry = definition.raytrace.rany ? definition.raytrace.rany->getEntryPointName() : nullptr;
            program.raytrace.rayClosest = makeBlob(definition.raytrace.rclosest);
            program.raytrace.rayClosetEntryMainEntry = definition.raytrace.rclosest ? definition.raytrace.rclosest->getEntryPointName() : nullptr;
            program.raytrace.rayGen = makeBlob(definition.raytrace.rgen);
            program.raytrace.rayGenEntryMainEntry = definition.raytrace.rgen ? definition.raytrace.rgen->getEntryPointName() : nullptr;
            program.raytrace.rayIntersect = makeBlob(definition.raytrace.rintersect);
            program.raytrace.rayIntersectMainEntry = definition.raytrace.rintersect ? definition.raytrace.rintersect->getEntryPointName() : nullptr;
            program.raytrace.rayMiss = makeBlob(definition.raytrace.rmiss);
            program.raytrace.rayMissMainEntry = definition.raytrace.rmiss ? definition.raytrace.rmiss->getEntryPointName() : nullptr;
            break;
        }
        default: return false;
    }
    g_shaderProgramMap[shaderProgram][permutation] = program;
    return true;
}


Bool isProgramCached(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation)
{
    auto& iter = g_shaderProgramMap.find(shaderProgram);
    if (iter != g_shaderProgramMap.end())
    {
        auto& permIter = iter->second.find(permutation);
        if (permIter != iter->second.end())
        {
            return true;
        }
    }
    return false;
}


Bool isProgramCached(ShaderProgramId shaderProgram)
{
    auto& iter = g_shaderProgramMap.find(shaderProgram);
    if (iter != g_shaderProgramMap.end())
    {
        return true;
    }
    return false;
}


D3DShaderProgram* obtainShaderProgram(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation)
{
    auto& iter = g_shaderProgramMap.find(shaderProgram);
    if (iter != g_shaderProgramMap.end())
    {
        auto& permIt = iter->second.find(permutation);
        if (permIt != iter->second.end())
        {
            return &permIt->second;
        }
    }
    return nullptr;
}


ResultCode loadNativeShaderProgramPermutation(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition)
{
    //R_ASSERT_FORMAT
    //    (
    //        (definition.intermediateCode == ShaderIntermediateCode_Dxil || definition.intermediateCode == ShaderIntermediateCode_Dxbc),
    //        "Shader intermediate is not supported by D3D"
    //    );
    if (isProgramCached(shaderProgram, permutation))
    {
        return RecluseResult_AlreadyExists;
    }
    Bool result = internalMakeShaderProgram(shaderProgram, permutation, definition);
    return result ? RecluseResult_Ok : RecluseResult_Failed;
}


ResultCode unloadAll()
{
    for (auto& iter : g_shaderProgramMap)
    {
        for (auto& program : iter.second)
        {
            destroyShaderProgram(&program.second);
        }
    }
    g_shaderProgramMap.clear();
    return RecluseResult_Ok;
}


ResultCode unloadPrograms(ShaderProgramId shaderProgram)
{
    auto& iter = g_shaderProgramMap.find(shaderProgram);
    if (iter != g_shaderProgramMap.end())
    {
        for (auto& perm : iter->second)
        {
            destroyShaderProgram(&perm.second);
        }
        iter->second.clear();
    }
    return RecluseResult_Ok;
}
} // Cache
} // D3D
} // Recluse