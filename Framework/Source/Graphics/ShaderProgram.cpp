//
#include "Recluse/Graphics/ShaderProgram.hpp"
#include "Recluse/Graphics/ShaderBuilder.hpp"

#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"

#include <unordered_set>

namespace Recluse {


std::unordered_set<VertexInputLayoutId> g_vertexLayoutIds;


struct ShaderProgramDatabaseHeader
{
    char    name[512];
    Hash64  nameHash;
    U32     nameSize;
    U32     version;
    U32     numPrograms;
    U32     numShaders;
};


struct ShaderProgramHeader
{
    ShaderProgramId programId;
    U32             numPermutations;
};


struct ShaderProgramPermutationHeader
{
    ShaderProgramPermutation    permutation;
    BindType                    bindType;
    ShaderIntermediateCode      intermediateCode;
    union 
    {
        struct
        {
            Hash64 vs;
            Hash64 ps;
            Hash64 gs;
            Hash64 hs;
            Hash64 ds;
        } graphics;
        struct
        {
            Hash64 cs;
        } compute;
        struct
        {
            Hash64 rgen;
            Hash64 rmiss;
            Hash64 rany;
            Hash64 rclosest;
            Hash64 rintersect;
        } raytrace;
    };
};


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
    auto& iter = m_shaderProgramMetaMap.find(shaderProgram);
    if (iter != m_shaderProgramMetaMap.end())
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
        auto& iter = m_shaderMap.find(shaderHash);
        if (iter != m_shaderMap.end())
        {
            U32 count = iter->second->release();
            if (count == 0)
            {
                Shader::destroy(iter->second);
                m_shaderMap.erase(iter);
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
    auto& iter = m_shaderProgramMetaMap.find(program);
    if (iter != m_shaderProgramMetaMap.end())
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
        m_shaderProgramMetaMap.erase(iter);
    }
    return RecluseResult_NotFound;
}


void ShaderProgramDatabase::storeShader(Shader* pShader)
{
    if (!pShader)
        return;
    Hash64 hash = makeShaderHash(pShader->getByteCode(), pShader->getSzBytes(), pShader->getPermutationId());
    auto& iter = m_shaderMap.find(hash);
    if (iter != m_shaderMap.end())
    {
        iter->second->addReference();
    }
    else
    {
        m_shaderMap.insert(std::make_pair(hash, pShader));
        //g_shaderMap[hash]->addReference();
    }
}


void ShaderProgramDatabase::storeShaderProgramDefinition(const ShaderProgramDefinition& definition, ShaderProgramId shaderProgram, ShaderProgramPermutation permutation)
{
    m_shaderProgramMetaMap[shaderProgram].insert(std::make_pair(permutation, definition));
    ShaderProgramDefinition& storedDefinition = m_shaderProgramMetaMap[shaderProgram][permutation];
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
    auto& iter = m_shaderMap.find(id);
    if (iter != m_shaderMap.end())
        return iter->second;
    return nullptr;
}


Hash64 ShaderProgramDatabase::makeShaderHash(const char* bytecode, U32 lengthBytes, ShaderProgramPermutation permutation)
{
    if (bytecode && (lengthBytes > 0))
    {
        Hash64 shaderHash = Shader::makeShaderHash(bytecode, lengthBytes);
        shaderHash ^= permutation;
        return shaderHash;
    }
    return 0;
}


void ShaderProgramDatabase::clearShaderProgramDefinitions()
{
    for (auto& iter : m_shaderProgramMetaMap)
    {
        for (auto& definition : iter.second)
        {
            cleanUpShaderProgramDefinition(definition.second);
        }
        iter.second.clear();
    }

    m_shaderProgramMetaMap.clear();
}


ResultCode ShaderProgramDatabase::serialize(Archive* pArchive)
{
    {
        ShaderProgramDatabaseHeader shaderProgramDatabaseHeader = { };
        memcpy(shaderProgramDatabaseHeader.name, (void*)m_name.data(), m_name.size());
        shaderProgramDatabaseHeader.nameHash = m_nameHash;
        shaderProgramDatabaseHeader.nameSize = static_cast<U32>(m_name.size());
        shaderProgramDatabaseHeader.version = 0;
        shaderProgramDatabaseHeader.numPrograms = static_cast<U32>(m_shaderProgramMetaMap.size());
        shaderProgramDatabaseHeader.numShaders = static_cast<U32>(m_shaderMap.size());
        pArchive->write(&shaderProgramDatabaseHeader, sizeof(ShaderProgramDatabaseHeader));
    }

    // Write shaders before we write the metamap.
    for (auto& shaderIter : m_shaderMap)
    {
        shaderIter.second->serialize(pArchive);
        U32 references = shaderIter.second->getReference();
        pArchive->write(&references, sizeof(U32));
    }

    for (auto& shaderProgram : m_shaderProgramMetaMap)
    {
        ShaderProgramHeader shaderProgramHeader = { };
        shaderProgramHeader.numPermutations = shaderProgram.second.size();
        shaderProgramHeader.programId = shaderProgram.first;
        pArchive->write(&shaderProgramHeader, sizeof(ShaderProgramHeader));

        for (auto& permIter : shaderProgram.second)
        {
            ShaderProgramDefinition& definition = permIter.second;
            ShaderProgramPermutation permutation = permIter.first;
            ShaderProgramPermutationHeader permHeader = { };
            permHeader.bindType = definition.pipelineType;
            permHeader.permutation = permutation;
            permHeader.intermediateCode = definition.intermediateCode;
            switch (permHeader.bindType)
            {
                case BindType_Graphics:
                {
                    permHeader.graphics.vs = definition.graphics.vs ? ShaderProgramDatabase::makeShaderHash(definition.graphics.vs->getByteCode(), definition.graphics.vs->getSzBytes(), permutation) : 0;
                    permHeader.graphics.ps = definition.graphics.ps ? ShaderProgramDatabase::makeShaderHash(definition.graphics.ps->getByteCode(), definition.graphics.ps->getSzBytes(), permutation) : 0;
                    permHeader.graphics.gs = definition.graphics.gs ? ShaderProgramDatabase::makeShaderHash(definition.graphics.gs->getByteCode(), definition.graphics.gs->getSzBytes(), permutation) : 0;
                    permHeader.graphics.ds = definition.graphics.ds ? ShaderProgramDatabase::makeShaderHash(definition.graphics.ds->getByteCode(), definition.graphics.ds->getSzBytes(), permutation) : 0;
                    permHeader.graphics.hs = definition.graphics.hs ? ShaderProgramDatabase::makeShaderHash(definition.graphics.hs->getByteCode(), definition.graphics.hs->getSzBytes(), permutation) : 0;
                    break;
                }
                case BindType_Compute:
                {
                    permHeader.compute.cs = definition.compute.cs ? ShaderProgramDatabase::makeShaderHash(definition.compute.cs->getByteCode(), definition.compute.cs->getSzBytes(), permutation) : 0;
                    break;
                }
                case BindType_RayTrace:
                {
                    permHeader.raytrace.rany = definition.raytrace.rany ? ShaderProgramDatabase::makeShaderHash(definition.raytrace.rany->getByteCode(), definition.raytrace.rany->getSzBytes(), permutation) : 0;
                    permHeader.raytrace.rclosest = definition.raytrace.rclosest ? ShaderProgramDatabase::makeShaderHash(definition.raytrace.rclosest->getByteCode(), definition.raytrace.rclosest->getSzBytes(), permutation) : 0;
                    permHeader.raytrace.rgen = definition.raytrace.rgen ? ShaderProgramDatabase::makeShaderHash(definition.raytrace.rgen->getByteCode(), definition.raytrace.rgen->getSzBytes(), permutation) : 0;
                    permHeader.raytrace.rintersect = definition.raytrace.rintersect ? ShaderProgramDatabase::makeShaderHash(definition.raytrace.rintersect->getByteCode(), definition.raytrace.rintersect->getSzBytes(), permutation) : 0;
                    permHeader.raytrace.rmiss = definition.raytrace.rmiss ? ShaderProgramDatabase::makeShaderHash(definition.raytrace.rmiss->getByteCode(), definition.raytrace.rmiss->getSzBytes(), permutation) : 0;
                    break;
                }
            }
            pArchive->write(&permHeader, sizeof(ShaderProgramPermutationHeader));
        }
    }
    
    return RecluseResult_Ok;
}


ResultCode ShaderProgramDatabase::deserialize(Archive* pArchive)
{
    ShaderProgramDatabaseHeader header = { };
    {
        pArchive->read(&header, sizeof(ShaderProgramDatabaseHeader));
        m_name.resize(header.nameSize);
        memcpy((void*)m_name.data(), header.name, m_name.size());
        m_nameHash = header.nameHash;       
    }

    for (U32 i = 0; i < header.numShaders; ++i)
    {
        Shader* shader = Shader::create();
        shader->deserialize(pArchive);
        U32 references = 0;
        pArchive->read(&references, sizeof(U32));
        shader->addReference(references-1);
        Hash64 shaderHash = ShaderProgramDatabase::makeShaderHash(shader->getByteCode(), shader->getSzBytes(), shader->getPermutationId());
        m_shaderMap.insert(std::make_pair(shaderHash, shader));
    }

    for (U32 i = 0; i < header.numPrograms; ++i)
    {
        ShaderProgramHeader shaderProgramHeader = { };
        pArchive->read(&shaderProgramHeader, sizeof(ShaderProgramHeader));
        m_shaderProgramMetaMap.insert(std::make_pair(shaderProgramHeader.programId, PermutationMap()));

        for (U32 permIt = 0; permIt < shaderProgramHeader.numPermutations; ++permIt)
        {
            ShaderProgramPermutationHeader permHeader = { };
            pArchive->read(&permHeader, sizeof(ShaderProgramPermutationHeader));
            ShaderProgramPermutation permutation = permHeader.permutation;
            ShaderProgramDefinition definition = { };
            definition.pipelineType = permHeader.bindType;
            definition.intermediateCode = permHeader.intermediateCode;
            switch (permHeader.bindType)
            {
                case BindType_Graphics:
                {
                    definition.graphics.vs = obtainShader(permHeader.graphics.vs);
                    definition.graphics.ps = obtainShader(permHeader.graphics.ps);
                    definition.graphics.gs = obtainShader(permHeader.graphics.gs);
                    definition.graphics.ds = obtainShader(permHeader.graphics.ds);
                    definition.graphics.hs = obtainShader(permHeader.graphics.hs);
                    break;
                }
                case BindType_Compute:
                {
                    definition.compute.cs = obtainShader(permHeader.compute.cs);
                    break;
                }
                case BindType_RayTrace:
                {
                    definition.raytrace.rany = obtainShader(permHeader.raytrace.rany);
                    definition.raytrace.rclosest = obtainShader(permHeader.raytrace.rclosest);
                    definition.raytrace.rgen = obtainShader(permHeader.raytrace.rgen);
                    definition.raytrace.rintersect = obtainShader(permHeader.raytrace.rintersect);
                    definition.raytrace.rmiss = obtainShader(permHeader.raytrace.rmiss);
                    break;
                }
            }
            m_shaderProgramMetaMap[shaderProgramHeader.programId].insert(std::make_pair(permutation, definition));
        }
    }

    return RecluseResult_Ok;
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