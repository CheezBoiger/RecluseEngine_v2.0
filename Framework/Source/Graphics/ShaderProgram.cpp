//
#include "Recluse/Graphics/ShaderProgramBuilder.hpp"
#include "Recluse/Graphics/ShaderBuilder.hpp"

#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include <unordered_set>

namespace Recluse {
namespace Builder {

R_INTERNAL std::unordered_map<ShaderProgramId, 
                    std::unordered_map<ShaderProgramPermutation, ShaderProgramDefinition>> g_shaderProgramMetaMap;
R_INTERNAL std::unordered_map<ShaderIntermediateCode, ShaderBuilder*> g_shaderBuilderMap;
R_INTERNAL std::unordered_set<VertexInputLayoutId> g_vertexLayoutIds;


ShaderProgramDescription::ShaderProgramDescription(const ShaderProgramDescription& description)
{
    pipelineType = description.pipelineType;
    permutationDefinitions = description.permutationDefinitions;
    language = description.language;
    switch (pipelineType)
    {
    case BindType_Compute:
        compute.cs = description.compute.cs;
        compute.csName = description.compute.csName;
        break;
    case BindType_Graphics:
        graphics.vs = description.graphics.vs;
        graphics.ps = description.graphics.ps;
        graphics.ds = description.graphics.ds;
        graphics.gs = description.graphics.gs;
        graphics.hs = description.graphics.hs;
        graphics.vsName = description.graphics.vsName;
        graphics.psName = description.graphics.psName;
        graphics.gsName = description.graphics.gsName;
        graphics.dsName = description.graphics.dsName;
        graphics.hsName = description.graphics.hsName;
        break;
    case BindType_RayTrace:
        raytrace.rany = description.raytrace.rany;
        raytrace.rclosest = description.raytrace.rclosest;
        raytrace.rgen = description.raytrace.rgen;
        raytrace.rintersect = description.raytrace.rintersect;
        raytrace.rmiss = description.raytrace.rmiss;
        raytrace.ranyName = description.raytrace.ranyName;
        raytrace.rclosestName = description.raytrace.rclosestName;
        raytrace.rgenName = description.raytrace.rgenName;
        raytrace.rintersectName = description.raytrace.rintersectName;
        raytrace.rmissName = description.raytrace.rmissName;
        break;
    }
}


ShaderProgramDescription::ShaderProgramDescription(ShaderProgramDescription&& description)
{
    pipelineType = description.pipelineType;
    permutationDefinitions = std::move(description.permutationDefinitions);
    language = description.language;
    switch (pipelineType)
    {
    case BindType_Compute:
        compute.cs = std::move(description.compute.cs);
        compute.csName = description.compute.csName;
        break;
    case BindType_Graphics:
        graphics.vs = std::move(description.graphics.vs);
        graphics.ps = std::move(description.graphics.ps);
        graphics.ds = std::move(description.graphics.ds);
        graphics.gs = std::move(description.graphics.gs);
        graphics.hs = std::move(description.graphics.hs);
        graphics.vsName = description.graphics.vsName;
        graphics.psName = description.graphics.psName;
        graphics.gsName = description.graphics.gsName;
        graphics.dsName = description.graphics.dsName;
        graphics.hsName = description.graphics.hsName;
        break;
    case BindType_RayTrace:
        raytrace.rany = std::move(description.raytrace.rany);
        raytrace.rclosest = std::move(description.raytrace.rclosest);
        raytrace.rgen = std::move(description.raytrace.rgen);
        raytrace.rintersect = std::move(description.raytrace.rintersect);
        raytrace.rmiss = std::move(description.raytrace.rmiss);
        raytrace.ranyName = description.raytrace.ranyName;
        raytrace.rclosestName = description.raytrace.rclosestName;
        raytrace.rgenName = description.raytrace.rgenName;
        raytrace.rintersectName = description.raytrace.rintersectName;
        raytrace.rmissName = description.raytrace.rmissName;
        break;
    }
}


ShaderProgramDefinition::ShaderProgramDefinition(const ShaderProgramDefinition& def)
{
    pipelineType = def.pipelineType;
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


static Shader* compileShader(ShaderBuilder* shaderBuilder, const char* entryPoint, const std::string& shaderPath, ShaderLang language, ShaderType shaderType, ErrType& errorOut)
{
    Shader* shader = nullptr;
    if (!shaderPath.empty())
    { 
        shader = Shader::create();
        FileBufferData file = { };
        ErrType error = File::readFrom(&file, shaderPath);
        if (error == RecluseResult_Ok)
        { 
            error = shaderBuilder->compile(shader, entryPoint, file.data(), file.size(), language, shaderType);
            if (error != RecluseResult_Ok)
            {
                Shader::destroy(shader);
                shader = nullptr;
            }
        }
        errorOut |= error;
    }
    return shader;
}

static ShaderProgramDefinition makeShaderProgramDefinition(const ShaderProgramDescription& description, ShaderBuilder* shaderBuilder, ErrType& errorOut)
{
    ShaderProgramDefinition definition;
    definition.pipelineType = description.pipelineType;
    const ShaderLang language = description.language;

    switch (definition.pipelineType)
    {
    case BindType_Compute:
        definition.compute.cs = compileShader(shaderBuilder, description.compute.csName, description.compute.cs, language, ShaderType_Compute, errorOut);
        break;
    case BindType_Graphics:
        definition.graphics.vs = compileShader(shaderBuilder, description.graphics.vsName, description.graphics.vs, language, ShaderType_Vertex, errorOut);
        definition.graphics.ps = compileShader(shaderBuilder, description.graphics.psName, description.graphics.ps, language, ShaderType_Pixel, errorOut);
        definition.graphics.gs = compileShader(shaderBuilder, description.graphics.gsName, description.graphics.gs, language, ShaderType_Geometry, errorOut);
        definition.graphics.hs = compileShader(shaderBuilder, description.graphics.hsName, description.graphics.hs, language, ShaderType_Hull, errorOut);
        definition.graphics.ds = compileShader(shaderBuilder, description.graphics.dsName, description.graphics.ds, language, ShaderType_Domain, errorOut);
        break;
    case BindType_RayTrace:
        definition.raytrace.rany = compileShader(shaderBuilder, description.raytrace.ranyName, description.raytrace.rany, language, ShaderType_RayAnyHit, errorOut);
        definition.raytrace.rclosest = compileShader(shaderBuilder, description.raytrace.rclosestName, description.raytrace.rclosest, language, ShaderType_RayClosestHit, errorOut);
        definition.raytrace.rgen = compileShader(shaderBuilder, description.raytrace.rgenName, description.raytrace.rgen, language, ShaderType_RayGeneration, errorOut);
        definition.raytrace.rintersect = compileShader(shaderBuilder, description.raytrace.rintersectName, description.raytrace.rintersect, language, ShaderType_RayIntersect, errorOut);
        definition.raytrace.rmiss = compileShader(shaderBuilder, description.raytrace.rmissName, description.raytrace.rmiss, language, ShaderType_RayMiss, errorOut);
        break;
    }

    if (errorOut != RecluseResult_Ok)
    {
        errorOut = RecluseResult_Failed;
    }

    return definition;
}

static void destroyShader(Shader* shader)
{
    if (shader)
    {
        Shader::destroy(shader);
    }
}

void destroyShaderProgramDefinition(ShaderProgramDefinition& definition)
{
    switch (definition.pipelineType)
    {
    case BindType_Compute:
        destroyShader(definition.compute.cs);
        break;
    case BindType_Graphics:
        destroyShader(definition.graphics.ds);
        destroyShader(definition.graphics.gs);
        destroyShader(definition.graphics.hs);
        destroyShader(definition.graphics.ps);
        destroyShader(definition.graphics.vs);
        break;
    case BindType_RayTrace:
        destroyShader(definition.raytrace.rany);
        destroyShader(definition.raytrace.rclosest);
        destroyShader(definition.raytrace.rgen);
        destroyShader(definition.raytrace.rintersect);
        destroyShader(definition.raytrace.rmiss);
        break;
    }
}

ErrType buildShaderProgramDefinitions(const ShaderProgramDescription& description, ShaderProgramId outId, ShaderIntermediateCode imm)
{
    if (g_shaderBuilderMap.find(imm) == g_shaderBuilderMap.end())
    {
        g_shaderBuilderMap[imm] = createGlslangShaderBuilder(imm);
        g_shaderBuilderMap[imm]->setUp();
    }

    ErrType result                  = RecluseResult_Ok;
    ShaderBuilder* shaderBuilder    = g_shaderBuilderMap[imm];
    R_ASSERT(shaderBuilder != NULL);

    auto makeInstanceFunc = [&description, shaderBuilder, outId] (ShaderProgramPermutation permutation) -> ErrType
    {
        ErrType result = RecluseResult_Ok;
        ShaderProgramDefinition definition = makeShaderProgramDefinition(description, shaderBuilder, result);
        if (result != RecluseResult_Ok)
        {
            destroyShaderProgramDefinition(definition);
        }
        else
        {
            g_shaderProgramMetaMap[outId].insert(std::make_pair(permutation, definition));
        }

        return result;
    };
 
    auto& programIt = g_shaderProgramMetaMap.find(outId);
    if (programIt == g_shaderProgramMetaMap.end())
    { 
        // Our first creation is the zeroth permutation.
        result = makeInstanceFunc(0);

        if (result == RecluseResult_Ok)
        {
            // If we have permutations, we can run them through here.
            for (U32 permIt = 0; permIt < description.permutationDefinitions.size(); ++permIt)
            {
                ShaderProgramPermutation permutation = 0;
                auto& permutationDefinition = description.permutationDefinitions[permIt];
                for (U32 defIt = 0; defIt < permutationDefinition.size(); ++defIt)
                {
                    const ShaderProgramPermutationDefinition& permDef = permutationDefinition[defIt];
                    permutation |= makeBitset64(permDef.offset, permDef.size, permDef.value);
                }

                result = makeInstanceFunc(permutation);
                if (result != RecluseResult_Ok)
                {
                    break;
                }
            }
        }
    }
    return result;
}


ShaderProgramDefinition* obtainShaderProgramDefinition(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation)
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


ErrType releaseShaderProgramDefinition(ShaderProgramId program)
{
    auto& iter = g_shaderProgramMetaMap.find(program);
    if (iter != g_shaderProgramMetaMap.end())
    {
        g_shaderProgramMetaMap.erase(iter);
        return RecluseResult_Ok;
    }
    return RecluseResult_NotFound;
}

void clearShaderProgramDefinitions()
{
    for (auto& iter : g_shaderProgramMetaMap)
    {
        for (auto& definition : iter.second)
        {
            destroyShaderProgramDefinition(definition.second);
        }

        iter.second.clear();
    }

    g_shaderProgramMetaMap.clear();
}

namespace Runtime {

ErrType buildShaderProgram(GraphicsDevice* pDevice, ShaderProgramId shaderProgram)
{
    ErrType e = RecluseResult_Ok;
    for (auto& definition : g_shaderProgramMetaMap[shaderProgram])
    {
        e = pDevice->loadShaderProgram(shaderProgram, definition.first, definition.second);
        if (e == RecluseResult_NeedsUpdate)
            continue;
        else if (e != RecluseResult_Ok)
            break;
    }

    return e;
}


ErrType buildAllShaderPrograms(GraphicsDevice* pDevice)
{
    ErrType e = RecluseResult_Ok;
    for (auto definitions : g_shaderProgramMetaMap)
    {
        e = buildShaderProgram(pDevice, definitions.first);
        if (e == RecluseResult_NeedsUpdate)
            continue;
        else if (e != RecluseResult_Ok)
            break;
    }
    return e;
}

ErrType releaseShaderProgram(GraphicsDevice* pDevice, ShaderProgramId shaderProgram)
{
    return pDevice->unloadShaderProgram(shaderProgram);
}


ErrType releaseAllShaderPrograms(GraphicsDevice* pDevice)
{
    pDevice->unloadAllShaderPrograms();
    return RecluseResult_Ok;
}


ErrType buildVertexInputLayout(GraphicsDevice* pDevice, const VertexInputLayout& layout, VertexInputLayoutId inputLayoutId)
{
    auto it = g_vertexLayoutIds.find(inputLayoutId);
    if (it != g_vertexLayoutIds.end())
    { 
        if (pDevice->makeVertexLayout(inputLayoutId, layout))
            return RecluseResult_Ok;
        else
            return RecluseResult_Failed;
    }
    else
    {
        return RecluseResult_Ok;
    }
}


ErrType releaseVertexInputLayout(GraphicsDevice* pDevice, VertexInputLayoutId inputLayoutId)
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

ErrType releaseAllVertexInputLayouts(GraphicsDevice* pDevice)
{
    ErrType err = RecluseResult_Ok;
    for (auto id : g_vertexLayoutIds)
    {
        err |= releaseVertexInputLayout(pDevice, id);
    }
    return err;
}
} // Realtime
} // Builder
} // Recluse