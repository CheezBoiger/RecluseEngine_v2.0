//
#include "Recluse/Graphics/ShaderProgram.hpp"
#include "Recluse/Graphics/ShaderBuilder.hpp"

#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include <unordered_set>

namespace Recluse {


std::unordered_set<VertexInputLayoutId> g_vertexLayoutIds;

ShaderProgramDefinition::ShaderProgramDefinition(const ShaderProgramDefinition& def)
{
    pipelineType = def.pipelineType;
    intermediateCode = def.intermediateCode;
    switch (def.pipelineType)
    {
    case BindType_Compute:
        compute.cs = def.compute.cs;
        break;
    case BindType_Graphics:
        graphics.ps = def.graphics.ps;
        graphics.vs = def.graphics.vs;
        graphics.gs = def.graphics.ds;
        graphics.hs = def.graphics.hs;
        graphics.ds = def.graphics.ds;
        break;
    case BindType_RayTrace:
        raytrace.rgen = def.raytrace.rgen;
        raytrace.rany = def.raytrace.rany;
        raytrace.rclosest = def.raytrace.rclosest;
        raytrace.rintersect = def.raytrace.rintersect;
        raytrace.rmiss = def.raytrace.rmiss;
        break;
    }
}


ShaderProgramDefinition::ShaderProgramDefinition(ShaderProgramDefinition&& def)
{
    pipelineType = def.pipelineType;
    intermediateCode = def.intermediateCode;
    switch (def.pipelineType)
    {
    case BindType_Compute:
        compute.cs = std::move(def.compute.cs);
        break;
    case BindType_Graphics:
        graphics.ps = std::move(def.graphics.ps);
        graphics.vs = std::move(def.graphics.vs);
        graphics.gs = std::move(def.graphics.ds);
        graphics.hs = std::move(def.graphics.hs);
        graphics.ds = std::move(def.graphics.ds);
        break;
    case BindType_RayTrace:
        raytrace.rgen = std::move(def.raytrace.rgen);
        raytrace.rany = std::move(def.raytrace.rany);
        raytrace.rclosest = std::move(def.raytrace.rclosest);
        raytrace.rintersect = std::move(def.raytrace.rintersect);
        raytrace.rmiss = std::move(def.raytrace.rmiss);
        break;
    }
}


ShaderProgramDefinition& ShaderProgramDefinition::operator=(const ShaderProgramDefinition& def)
{
    pipelineType = def.pipelineType;
    intermediateCode = def.intermediateCode;
    switch (def.pipelineType)
    {
    case BindType_Compute:
        compute.cs = def.compute.cs;
        break;
    case BindType_Graphics:
        graphics.ps = def.graphics.ps;
        graphics.vs = def.graphics.vs;
        graphics.gs = def.graphics.ds;
        graphics.hs = def.graphics.hs;
        graphics.ds = def.graphics.ds;
        break;
    case BindType_RayTrace:
        raytrace.rgen = def.raytrace.rgen;
        raytrace.rany = def.raytrace.rany;
        raytrace.rclosest = def.raytrace.rclosest;
        raytrace.rintersect = def.raytrace.rintersect;
        raytrace.rmiss = def.raytrace.rmiss;
        break;
    }
    return (*this);
}


ShaderProgramDefinition& ShaderProgramDefinition::operator=(ShaderProgramDefinition&& def)
{
    pipelineType = def.pipelineType;
    intermediateCode = def.intermediateCode;
    switch (def.pipelineType)
    {
    case BindType_Compute:
        compute.cs = std::move(def.compute.cs);
        break;
    case BindType_Graphics:
        graphics.ps = std::move(def.graphics.ps);
        graphics.vs = std::move(def.graphics.vs);
        graphics.gs = std::move(def.graphics.ds);
        graphics.hs = std::move(def.graphics.hs);
        graphics.ds = std::move(def.graphics.ds);
        break;
    case BindType_RayTrace:
        raytrace.rgen = std::move(def.raytrace.rgen);
        raytrace.rany = std::move(def.raytrace.rany);
        raytrace.rclosest = std::move(def.raytrace.rclosest);
        raytrace.rintersect = std::move(def.raytrace.rintersect);
        raytrace.rmiss = std::move(def.raytrace.rmiss);
        break;
    }
    return (*this);
}


ShaderProgramDefinition* ShaderProgramDatabase::obtainShaderProgramDefinition(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation)
{
    auto& iter = g_shaderProgramMetaMap.find(shaderProgram);
    if (iter != g_shaderProgramMetaMap.end())
    {
        auto& permIter = iter->second.find(permutation);
        if (permIter != iter->second.end())
        {
            return &permIter->second;
        }
    }

    return nullptr;
}


void ShaderProgramDatabase::removeShader(Shader* pShader)
{
    if (pShader)
    {
        Hash64 shaderHash = makeShaderHash(pShader->getByteCode(), pShader->getSzBytes(), pShader->getPermutationId());
        auto& iter = g_shaderMap.find(shaderHash);
        if (iter != g_shaderMap.end())
        {
            U32 count = iter->second->release();
            if (count == 0)
            {
                Shader::destroy(iter->second);
                g_shaderMap.erase(iter);
            }
        }
    }
}


void ShaderProgramDatabase::cleanUpShaderProgramDefinition(const ShaderProgramDefinition& definition)
{
    switch (definition.pipelineType)
    {
        case BindType_Graphics:
        {
            removeShader(definition.graphics.vs);
            removeShader(definition.graphics.ps);
            removeShader(definition.graphics.gs);
            removeShader(definition.graphics.ds);
            removeShader(definition.graphics.hs);
            break;
        }
        case BindType_Compute:
        {
            removeShader(definition.compute.cs);
            break;
        }
        case BindType_RayTrace:
        {
            removeShader(definition.raytrace.rany);
            removeShader(definition.raytrace.rclosest);
            removeShader(definition.raytrace.rgen);
            removeShader(definition.raytrace.rintersect);
            removeShader(definition.raytrace.rmiss);
            break;
        }
    }
}


ResultCode ShaderProgramDatabase::releaseShaderProgramDefinition(ShaderProgramId program, ShaderProgramPermutation permutation)
{
    auto& iter = g_shaderProgramMetaMap.find(program);
    if (iter != g_shaderProgramMetaMap.end())
    {
        auto& permIter = iter->second.find(permutation);
        if (permIter != iter->second.end())
        {
            cleanUpShaderProgramDefinition(permIter->second);
            iter->second.erase(permIter);
            return RecluseResult_Ok;
        }
    }
    if (iter->second.empty())
    {
        g_shaderProgramMetaMap.erase(iter);
    }
    return RecluseResult_NotFound;
}


void ShaderProgramDatabase::storeShader(Shader* pShader)
{
    if (!pShader)
        return;
    Hash64 hash = makeShaderHash(pShader->getByteCode(), pShader->getSzBytes(), pShader->getPermutationId());
    auto& iter = g_shaderMap.find(hash);
    if (iter != g_shaderMap.end())
    {
        iter->second->addReference();
    }
    else
    {
        g_shaderMap.insert(std::make_pair(hash, pShader));
        //g_shaderMap[hash]->addReference();
    }
}


void ShaderProgramDatabase::storeShaderProgramDefinition(const ShaderProgramDefinition& definition, ShaderProgramId shaderProgram, ShaderProgramPermutation permutation)
{
    g_shaderProgramMetaMap[shaderProgram].insert(std::make_pair(permutation, definition));
    ShaderProgramDefinition& storedDefinition = g_shaderProgramMetaMap[shaderProgram][permutation];
    switch (storedDefinition.pipelineType)
    {
        case BindType_Graphics:
        {
            storeShader(storedDefinition.graphics.vs);
            storeShader(storedDefinition.graphics.ps);
            storeShader(storedDefinition.graphics.gs);
            storeShader(storedDefinition.graphics.ds);
            storeShader(storedDefinition.graphics.hs);
            break;
        }
        case BindType_Compute:
        {
            storeShader(storedDefinition.compute.cs);
            break;
        }    
        case BindType_RayTrace:
        {
            storeShader(storedDefinition.raytrace.rany);
            storeShader(storedDefinition.raytrace.rclosest);
            storeShader(storedDefinition.raytrace.rgen);
            storeShader(storedDefinition.raytrace.rintersect);
            storeShader(storedDefinition.raytrace.rmiss);
            break;
        }
    }
}


Shader* ShaderProgramDatabase::obtainShader(Hash64 id) const
{
    auto& iter = g_shaderMap.find(id);
    if (iter != g_shaderMap.end())
        return iter->second;
    return nullptr;
}


Hash64 ShaderProgramDatabase::makeShaderHash(const char* bytecode, U32 lengthBytes, ShaderProgramPermutation permutation)
{
    if (bytecode)
    {
        Hash64 shaderHash = Shader::makeShaderHash(bytecode, lengthBytes);
        shaderHash ^= permutation;
        return shaderHash;
    }
    return 0;
}


void ShaderProgramDatabase::clearShaderProgramDefinitions()
{
    for (auto& iter : g_shaderProgramMetaMap)
    {
        for (auto& definition : iter.second)
        {
            cleanUpShaderProgramDefinition(definition.second);
        }
        iter.second.clear();
    }

    g_shaderProgramMetaMap.clear();
}

namespace Runtime {

ResultCode buildShaderProgram(GraphicsDevice* pDevice, const ShaderProgramDatabase& db, ShaderProgramId shaderProgram)
{
    ResultCode e = RecluseResult_Ok;
    const ShaderProgramDatabase::PermutationMap* dbmap = db.obtainShaderProgramPermutations(shaderProgram);
    for (const auto& definition : *dbmap)
    {
        e = pDevice->loadShaderProgram(shaderProgram, definition.first, definition.second);
        if (e == RecluseResult_NeedsUpdate)
            continue;
        else if (e != RecluseResult_Ok)
            break;
    }

    return e;
}


ResultCode buildAllShaderPrograms(GraphicsDevice* pDevice, const ShaderProgramDatabase& db)
{
    ResultCode e = RecluseResult_Ok;
    const ShaderProgramDatabase::MetaMap& metamap = db.obtainMetaMap();
    for (auto& definitions : metamap)
    {
        e = buildShaderProgram(pDevice, db, definitions.first);
        if (e == RecluseResult_NeedsUpdate)
            continue;
        else if (e != RecluseResult_Ok)
            break;
    }
    return e;
}

ResultCode releaseShaderProgram(GraphicsDevice* pDevice, ShaderProgramId shaderProgram)
{
    return pDevice->unloadShaderProgram(shaderProgram);
}


ResultCode releaseAllShaderPrograms(GraphicsDevice* pDevice)
{
    pDevice->unloadAllShaderPrograms();
    return RecluseResult_Ok;
}


ResultCode buildVertexInputLayout(GraphicsDevice* pDevice, const VertexInputLayout& layout, VertexInputLayoutId inputLayoutId)
{
    auto it = g_vertexLayoutIds.find(inputLayoutId);
    if (it != g_vertexLayoutIds.end())
    { 
        return RecluseResult_Ok;
    }
    else
    {
        if (pDevice->makeVertexLayout(inputLayoutId, layout))
        {
            g_vertexLayoutIds.insert(inputLayoutId);
            return RecluseResult_Ok;
        }
        else
            return RecluseResult_Failed;
    }
    return RecluseResult_Unexpected;
}


ResultCode releaseVertexInputLayout(GraphicsDevice* pDevice, VertexInputLayoutId inputLayoutId)
{
    auto it = g_vertexLayoutIds.find(inputLayoutId);
    if (it != g_vertexLayoutIds.end())
    { 
        if (pDevice->destroyVertexLayout(inputLayoutId))
        {
            g_vertexLayoutIds.erase(it);
            return RecluseResult_Ok;
        }
        else
            return RecluseResult_Failed;
    }
    return RecluseResult_NotFound;
}

ResultCode releaseAllVertexInputLayouts(GraphicsDevice* pDevice)
{
    ResultCode err = RecluseResult_Ok;
    for (auto id : g_vertexLayoutIds)
    {
        err |= releaseVertexInputLayout(pDevice, id);
    }
    return err;
}
} // Runtime
} // Recluse