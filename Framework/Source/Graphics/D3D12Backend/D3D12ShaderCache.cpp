//
#include "Recluse/Messaging.hpp"
#include "D3D12ShaderCache.hpp"

#include <unordered_map>
#include <map>

namespace Recluse {
namespace D3D {
namespace Cache {


std::unordered_map<ShaderProgramId, std::unordered_map<ShaderProgramPermutation, D3DShaderProgram>> g_shaderProgramMap;
std::unordered_map<ShaderProgramId, std::unordered_map<ShaderProgramPermutation, ShaderProgramReflection>> g_shaderProgramReflectionMap;
std::map<Hash64, std::map<ShaderProgramPermutation, ReferenceCounter<D3DShaderBytecode*>>> g_cachedShaderBlobs;


R_INTERNAL
D3DShaderBytecode* createBlob(SizeT sizeBytes, Hash64 shaderHash, ShaderPermutationId permutation)
{
    D3DShaderBytecode* pBlob = new D3DShaderBytecode();
    pBlob->ptr = nullptr;
    pBlob->sizeBytes = sizeBytes;
    pBlob->shaderId = shaderHash;
    pBlob->permutation = permutation;
    if (sizeBytes)
    {
        pBlob->ptr = malloc(sizeBytes);
    }
    return pBlob;
}


R_INTERNAL
void internalFreeBlob(D3DShaderBytecode* pBlob)
{
    if (pBlob)
    {
        if (pBlob->ptr)
            free(pBlob->ptr);
        delete pBlob;
    }
}


R_INTERNAL
D3DShaderBytecode* makeBlob(Shader* pShader)
{
    D3DShaderBytecode* pOutput = nullptr;
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
                permIter->second.addReference();
                pOutput = permIter->second();
            }
        }
        else
        {
            D3DShaderBytecode* pBlob = createBlob(pShader->getSzBytes(), shaderHash, permutation);
            memcpy(pBlob->ptr, pShader->getByteCode(), pShader->getSzBytes());
            g_cachedShaderBlobs[shaderHash][permutation] = pBlob;
            g_cachedShaderBlobs[shaderHash][permutation].addReference();
            pOutput = pBlob;
        }
    }
    return pOutput;
}


R_INTERNAL
void callReleaseBlob(D3DShaderBytecode* pBlob)
{
    if (pBlob)
    {
        auto it = g_cachedShaderBlobs.find(pBlob->shaderId);
        if (it != g_cachedShaderBlobs.end())
        {
            auto permIt = it->second.find(pBlob->permutation);
            if (permIt != it->second.end())
            {
                U32 refs = permIt->second.release();
                if (refs == 0)
                {
                    // We must destroy this bytecode for real. No one else is referencing it.
                    it->second.erase(permIt);
                    if (it->second.empty())
                        g_cachedShaderBlobs.erase(it);
                    // Destroy the actual bytecode blob.
                    internalFreeBlob(pBlob);
                }
            }
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
            if (pProgram->graphics.usesMeshShaders)
            {
                callReleaseBlob(pProgram->graphics.asBytecode);
                callReleaseBlob(pProgram->graphics.msBytecode);
            }
            else
            {
                callReleaseBlob(pProgram->graphics.vsBytecode);
                callReleaseBlob(pProgram->graphics.hsBytecode);
                callReleaseBlob(pProgram->graphics.dsBytecode);
                callReleaseBlob(pProgram->graphics.gsBytecode);
            }
            callReleaseBlob(pProgram->graphics.psBytecode);
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
Bool internalMakeShaderProgram(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition)
{
    D3DShaderProgram program    = { };
    program.bindType            = definition.pipelineType;
    switch (definition.pipelineType)
    {
        case BindType_Graphics:
        {
            program.graphics.usesMeshShaders = definition.graphics.usesMeshShaders;
            if (program.graphics.usesMeshShaders)
            {
                program.graphics.asBytecode = makeBlob(definition.graphics.as);
                program.graphics.asMainEntry = definition.graphics.as ? definition.graphics.as->getEntryPointName() : nullptr;
                program.graphics.msBytecode = makeBlob(definition.graphics.ms);
                program.graphics.msMainEntry = definition.graphics.ms ? definition.graphics.ms->getEntryPointName() : nullptr;
            }
            else
            {
                program.graphics.vsBytecode = makeBlob(definition.graphics.vs);
                program.graphics.vsMainEntry = definition.graphics.vs ? definition.graphics.vs->getEntryPointName() : nullptr;
                program.graphics.gsBytecode = makeBlob(definition.graphics.gs);
                program.graphics.gsMainEntry = definition.graphics.gs ? definition.graphics.gs->getEntryPointName() : nullptr;
                program.graphics.hsBytecode = makeBlob(definition.graphics.hs);
                program.graphics.hsMainEntry = definition.graphics.hs ? definition.graphics.hs->getEntryPointName() : nullptr;
                program.graphics.dsBytecode = makeBlob(definition.graphics.ds);
                program.graphics.dsMainEntry = definition.graphics.ds ? definition.graphics.ds->getEntryPointName() : nullptr;
            }
            program.graphics.psBytecode = makeBlob(definition.graphics.ps);
            program.graphics.psMainEntry = definition.graphics.ps ? definition.graphics.ps->getEntryPointName() : nullptr;
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


ResultCode loadNativeShaderProgramPermutation(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition)
{
    R_ASSERT_FORMAT
        (
            (definition.intermediateCode == ShaderIntermediateCode_Dxil || definition.intermediateCode == ShaderIntermediateCode_Dxbc),
            "Shader intermediate code from program is not supported by D3D"
        );
    if (isProgramCached(shaderProgram, permutation))
    {
        return RecluseResult_AlreadyExists;
    }
    Bool result = internalMakeShaderProgram(shaderProgram, permutation, definition);

    if (result == RecluseResult_Ok)
    {
        auto& table = g_shaderProgramReflectionMap[shaderProgram];
        auto it = table.find(permutation);
        if (it == table.end())
        {
            table[permutation] = definition.programReflection;
        }
    }

    return result ? RecluseResult_Ok : RecluseResult_Failed;
}


ShaderProgramReflection* obtainShaderProgramReflection(ShaderProgramId programId, ShaderProgramPermutation permutation)
{
    auto& table = g_shaderProgramReflectionMap[programId];
    auto it = table.find(permutation);
    if (it != table.end())
    {
        return &it->second;
    }
    return nullptr;
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