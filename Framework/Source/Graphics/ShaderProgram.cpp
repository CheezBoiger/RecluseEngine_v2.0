//
#include "Recluse/Graphics/ShaderProgram.hpp"

#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"

#include <unordered_set>
#include <iterator>

namespace Recluse {


std::unordered_set<VertexInputLayoutId> g_vertexLayoutIds;
const U32                               kCurrentShaderProgramDatabaseVersion = 0;

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
    U64                         reflectionInfoCount;
    union 
    {
        struct
        {
            Bool64 usesMeshShaders;
            union
            {
                struct
                {
                    Hash64 vs;
                    Hash64 gs;
                    Hash64 hs;
                    Hash64 ds;
                };
                struct
                {
                    Hash64 as;
                    Hash64 ms;
                };
            };
            Hash64 ps;
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
        graphics.usesMeshShaders = def.graphics.usesMeshShaders;
        if (graphics.usesMeshShaders)
        {
            graphics.as = def.graphics.as;
            graphics.ms = def.graphics.ms;
        }
        else
        {
            graphics.vs = def.graphics.vs;
            graphics.gs = def.graphics.ds;
            graphics.hs = def.graphics.hs;
            graphics.ds = def.graphics.ds;
        }
        graphics.ps = def.graphics.ps;
        break;
    case BindType_RayTrace:
        raytrace.rgen = def.raytrace.rgen;
        raytrace.rany = def.raytrace.rany;
        raytrace.rclosest = def.raytrace.rclosest;
        raytrace.rintersect = def.raytrace.rintersect;
        raytrace.rmiss = def.raytrace.rmiss;
        break;
    }
    reflectionInfo.insert(def.reflectionInfo.begin(), def.reflectionInfo.end());
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
        graphics.usesMeshShaders = def.graphics.usesMeshShaders;
        if (graphics.usesMeshShaders)
        {
            graphics.as = std::move(def.graphics.as);
            graphics.ms = std::move(def.graphics.ms);
        }
        else
        {
            graphics.vs = std::move(def.graphics.vs);
            graphics.gs = std::move(def.graphics.gs);
            graphics.hs = std::move(def.graphics.hs);
            graphics.ds = std::move(def.graphics.ds);
        }
        graphics.ps = std::move(def.graphics.ps);
        break;
    case BindType_RayTrace:
        raytrace.rgen = std::move(def.raytrace.rgen);
        raytrace.rany = std::move(def.raytrace.rany);
        raytrace.rclosest = std::move(def.raytrace.rclosest);
        raytrace.rintersect = std::move(def.raytrace.rintersect);
        raytrace.rmiss = std::move(def.raytrace.rmiss);
        break;
    }
    reflectionInfo = std::move(def.reflectionInfo);
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
        graphics.usesMeshShaders = def.graphics.usesMeshShaders;
        if (graphics.usesMeshShaders)
        {
            graphics.as = def.graphics.as;
            graphics.ms = def.graphics.ms;
        }
        else
        {
            graphics.vs = def.graphics.vs;
            graphics.gs = def.graphics.gs;
            graphics.hs = def.graphics.hs;
            graphics.ds = def.graphics.ds;
        }
        graphics.ps = def.graphics.ps;
        break;
    case BindType_RayTrace:
        raytrace.rgen = def.raytrace.rgen;
        raytrace.rany = def.raytrace.rany;
        raytrace.rclosest = def.raytrace.rclosest;
        raytrace.rintersect = def.raytrace.rintersect;
        raytrace.rmiss = def.raytrace.rmiss;
        break;
    }
    reflectionInfo.insert(def.reflectionInfo.begin(), def.reflectionInfo.end());
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
        graphics.usesMeshShaders = def.graphics.usesMeshShaders;
        if (graphics.usesMeshShaders)
        {
            graphics.as = std::move(def.graphics.as);
            graphics.ms = std::move(def.graphics.ms);
        }
        else
        {
            graphics.vs = std::move(def.graphics.vs);
            graphics.gs = std::move(def.graphics.gs);
            graphics.hs = std::move(def.graphics.hs);
            graphics.ds = std::move(def.graphics.ds);
        }
        graphics.ps = std::move(def.graphics.ps);
        break;
    case BindType_RayTrace:
        raytrace.rgen = std::move(def.raytrace.rgen);
        raytrace.rany = std::move(def.raytrace.rany);
        raytrace.rclosest = std::move(def.raytrace.rclosest);
        raytrace.rintersect = std::move(def.raytrace.rintersect);
        raytrace.rmiss = std::move(def.raytrace.rmiss);
        break;
    }
    reflectionInfo = std::move(def.reflectionInfo);
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
            if (definition.graphics.usesMeshShaders)
            {
                removeShader(definition.graphics.as);
                removeShader(definition.graphics.ms);
            }
            else
            {
                removeShader(definition.graphics.vs);
                removeShader(definition.graphics.gs);
                removeShader(definition.graphics.ds);
                removeShader(definition.graphics.hs);
            }
            removeShader(definition.graphics.ps);
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
            if (storedDefinition.graphics.usesMeshShaders)
            {
                storeShader(storedDefinition.graphics.as);
                storeShader(storedDefinition.graphics.ms);
            }
            else
            {
                storeShader(storedDefinition.graphics.vs);
                storeShader(storedDefinition.graphics.gs);
                storeShader(storedDefinition.graphics.ds);
                storeShader(storedDefinition.graphics.hs);
            }
            storeShader(storedDefinition.graphics.ps);
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


ResultCode ShaderProgramDatabase::serialize(Archive* pArchive) const
{
    {
        ShaderProgramDatabaseHeader shaderProgramDatabaseHeader = { };
        memcpy(shaderProgramDatabaseHeader.name, (void*)m_name.data(), m_name.size());
        shaderProgramDatabaseHeader.nameHash = m_nameHash;
        shaderProgramDatabaseHeader.nameSize = static_cast<U32>(m_name.size());
        shaderProgramDatabaseHeader.version = kCurrentShaderProgramDatabaseVersion;
        shaderProgramDatabaseHeader.numPrograms = static_cast<U32>(m_shaderProgramMetaMap.size());
        shaderProgramDatabaseHeader.numShaders = static_cast<U32>(m_shaderMap.size());
        pArchive->write(&shaderProgramDatabaseHeader, sizeof(ShaderProgramDatabaseHeader));
    }

    // Write shaders before we write the metamap. Do this first after reading the header! Then we will iterate over all 
    // shader program definitions to write.
    for (auto& shaderIter : m_shaderMap)
    {
        shaderIter.second->serialize(pArchive);
        U32 references = shaderIter.second->getReference();
        pArchive->write(&references, sizeof(U32));
    }

    // Now we store the information of each shader program definition!
    for (auto& shaderProgram : m_shaderProgramMetaMap)
    {
        ShaderProgramHeader shaderProgramHeader = { };
        shaderProgramHeader.numPermutations = shaderProgram.second.size();
        shaderProgramHeader.programId = shaderProgram.first;
        pArchive->write(&shaderProgramHeader, sizeof(ShaderProgramHeader));

        // Store any 
        for (const auto& permIter : shaderProgram.second)
        {
            const ShaderProgramDefinition& definition = permIter.second;
            ShaderProgramPermutation permutation = permIter.first;
            ShaderProgramPermutationHeader permHeader = { };
            permHeader.bindType = definition.pipelineType;
            permHeader.permutation = permutation;
            permHeader.intermediateCode = definition.intermediateCode;
            permHeader.reflectionInfoCount = definition.reflectionInfo.size();

            // Finally, store all hash shader data into the shader program.
            switch (permHeader.bindType)
            {
                case BindType_Graphics:
                {
                    permHeader.graphics.usesMeshShaders = definition.graphics.usesMeshShaders;
                    if (permHeader.graphics.usesMeshShaders)
                    {
                        permHeader.graphics.as = definition.graphics.as ? ShaderProgramDatabase::makeShaderHash(definition.graphics.as->getByteCode(), definition.graphics.as->getSzBytes(), permutation) : 0;
                        permHeader.graphics.ms = definition.graphics.ms ? ShaderProgramDatabase::makeShaderHash(definition.graphics.ms->getByteCode(), definition.graphics.ms->getSzBytes(), permutation) : 0;
                    }
                    else
                    {
                        permHeader.graphics.vs = definition.graphics.vs ? ShaderProgramDatabase::makeShaderHash(definition.graphics.vs->getByteCode(), definition.graphics.vs->getSzBytes(), permutation) : 0;
                        permHeader.graphics.gs = definition.graphics.gs ? ShaderProgramDatabase::makeShaderHash(definition.graphics.gs->getByteCode(), definition.graphics.gs->getSzBytes(), permutation) : 0;
                        permHeader.graphics.ds = definition.graphics.ds ? ShaderProgramDatabase::makeShaderHash(definition.graphics.ds->getByteCode(), definition.graphics.ds->getSzBytes(), permutation) : 0;
                        permHeader.graphics.hs = definition.graphics.hs ? ShaderProgramDatabase::makeShaderHash(definition.graphics.hs->getByteCode(), definition.graphics.hs->getSzBytes(), permutation) : 0;
                    }
                    permHeader.graphics.ps = definition.graphics.ps ? ShaderProgramDatabase::makeShaderHash(definition.graphics.ps->getByteCode(), definition.graphics.ps->getSzBytes(), permutation) : 0;
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

            // If we have any reflection data. Loop over and store after the header is stored. This will help us get the header info first, to 
            // iterate any needed reflection data.
            for (const auto& reflectionInfo : definition.reflectionInfo)
            {
                ShaderType shaderType = reflectionInfo.first;
                const ShaderReflection* infoPtr = &reflectionInfo.second;
                pArchive->write(&shaderType, sizeof(ShaderType));
                infoPtr->serialize(pArchive);
            }
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

    if (header.version != kCurrentShaderProgramDatabaseVersion)
    {
        return RecluseResult_InvalidVersion;
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

            // Then check the core program definition and shader data.
            switch (permHeader.bindType)
            {
                case BindType_Graphics:
                {
                    definition.graphics.usesMeshShaders = !!permHeader.graphics.usesMeshShaders;
                    if (permHeader.graphics.usesMeshShaders)
                    {
                        definition.graphics.as = obtainShader(permHeader.graphics.as);
                        definition.graphics.ms = obtainShader(permHeader.graphics.ms);
                    }
                    else
                    {
                        definition.graphics.vs = obtainShader(permHeader.graphics.vs);
                        definition.graphics.gs = obtainShader(permHeader.graphics.gs);
                        definition.graphics.ds = obtainShader(permHeader.graphics.ds);
                        definition.graphics.hs = obtainShader(permHeader.graphics.hs);
                    }
                    definition.graphics.ps = obtainShader(permHeader.graphics.ps);
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

            // Deserialize reflection data for the shader program.
            for (U32 i = 0; i < permHeader.reflectionInfoCount; ++i)
            {
                ShaderType shaderType = ShaderType_None;
                ShaderReflection reflectionInfo = { };
                pArchive->read(&shaderType, sizeof(ShaderType));
                reflectionInfo.deserialize(pArchive);
                definition.reflectionInfo[shaderType] = reflectionInfo;
            }

            // Store the final result.
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