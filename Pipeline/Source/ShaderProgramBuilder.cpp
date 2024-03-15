//
#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Messaging.hpp"
#include <unordered_map>
#include <unordered_set>
#include <set>

R_DECLARE_GLOBAL_STRING(g_shaderBuilderName, "glslang", "ShaderBuilder.NameId");

namespace Recluse {
namespace Pipeline {
namespace Builder {


std::unordered_map<ShaderIntermediateCode, ShaderBuilder*> g_shaderBuilderMap;

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
        graphics.usesMeshShaders = description.graphics.usesMeshShaders;
        if (graphics.usesMeshShaders)
        {
            graphics.as = description.graphics.as;
            graphics.ms = description.graphics.ms;
            graphics.asName = description.graphics.asName;
            graphics.msName = description.graphics.msName;
        }
        else
        {
            graphics.vs = description.graphics.vs;
            graphics.vsName = description.graphics.vsName;
            graphics.ds = description.graphics.ds;
            graphics.gs = description.graphics.gs;
            graphics.hs = description.graphics.hs;
            graphics.gsName = description.graphics.gsName;
            graphics.dsName = description.graphics.dsName;
            graphics.hsName = description.graphics.hsName;
        }
        graphics.ps = description.graphics.ps;
        graphics.psName = description.graphics.psName;
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
        graphics.usesMeshShaders = description.graphics.usesMeshShaders;
        if (graphics.usesMeshShaders)
        {
            graphics.as = std::move(description.graphics.as);
            graphics.ms = std::move(description.graphics.ms);
            graphics.asName = description.graphics.asName;
            graphics.msName = description.graphics.msName;
        }
        else
        {
            graphics.vs = std::move(description.graphics.vs);
            graphics.ds = std::move(description.graphics.ds);
            graphics.gs = std::move(description.graphics.gs);
            graphics.hs = std::move(description.graphics.hs);
            graphics.vsName = description.graphics.vsName;
            graphics.gsName = description.graphics.gsName;
            graphics.dsName = description.graphics.dsName;
            graphics.hsName = description.graphics.hsName;
        }
        graphics.ps = std::move(description.graphics.ps);
        graphics.psName = description.graphics.psName;
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


static void destroyShader(ShaderProgramDatabase& db, Shader* shader)
{
    if (shader)
    {
        U32 count = shader->release();
        if (count == 0)
        {
            Shader::destroy(shader);
        }
    }
}


void destroyShaderProgramDefinition(ShaderProgramDatabase& db, ShaderProgramDefinition& definition)
{
    switch (definition.pipelineType)
    {
    case BindType_Compute:
        destroyShader(db, definition.compute.cs);
        break;
    case BindType_Graphics:
        destroyShader(db, definition.graphics.ps);
        if (definition.graphics.usesMeshShaders)
        {
            destroyShader(db, definition.graphics.as);
            destroyShader(db, definition.graphics.ms);
        }
        else
        {
            destroyShader(db, definition.graphics.vs);
            destroyShader(db, definition.graphics.ds);
            destroyShader(db, definition.graphics.gs);
            destroyShader(db, definition.graphics.hs);
        }
        break;
    case BindType_RayTrace:
        destroyShader(db, definition.raytrace.rany);
        destroyShader(db, definition.raytrace.rclosest);
        destroyShader(db, definition.raytrace.rgen);
        destroyShader(db, definition.raytrace.rintersect);
        destroyShader(db, definition.raytrace.rmiss);
        break;
    }
}


R_INTERNAL Shader* compileShader
    (
        ShaderBuilder* shaderBuilder, 
        ShaderProgramDatabase& db, 
        std::map<ShaderType, ShaderReflection>& reflectionOut,
        const char* entryPoint, 
        const std::string& shaderPath, 
        ShaderPermutationId permutation,
        ShaderLanguage language, 
        ShaderType shaderType,
        const std::vector<PreprocessDefine>& defines,
        ResultCode& errorOut
    )
{
    Shader* shader = nullptr;
    if (!shaderPath.empty())
    {
        FileBufferData file = { };
        ResultCode error = File::readFrom(&file, shaderPath);
        Hash64 shaderHash = db.makeShaderHash(file.data(), sizeof(char) * file.size());
        if (!db.hasShader(shaderHash))
        {
            shader = Shader::create();
            if (error == RecluseResult_Ok)
            { 
                error = shaderBuilder->compile(shader, entryPoint, file.data(), file.size(), language, shaderType, defines);
                if (error != RecluseResult_Ok)
                {
                    Shader::destroy(shader);
                    shader = nullptr;
                }
                else
                {
                    ShaderReflection reflectionData = { };
                    shader->setName(shaderPath.c_str());
                    ResultCode reflectError = shaderBuilder->reflect(reflectionData, shader->getByteCode(), shader->getSzBytes(), language);
                    if (reflectError == RecluseResult_Ok)
                    {
                        R_DEBUG("ShaderBuilder", "ShaderName: %s \nCBVs: %d\nSRVs:%d\nUAVs:%d\nSamplers: %d", 
                            shader->getName(), reflectionData.metadata.numCbvs, reflectionData.metadata.numSrvs, 
                            reflectionData.metadata.numUavs, reflectionData.metadata.numSamplers);
                            reflectionOut.insert(std::make_pair(shaderType, reflectionData));
                    }
                }
            }
        }
        else
        {
            shader = db.obtainShader(shaderHash);
        }
        errorOut |= error;
    }
    return shader;
}


R_INTERNAL 
ShaderProgramDefinition makeShaderProgramDefinition(ShaderProgramDatabase& db, const ShaderProgramDescription& description, const ShaderProgramPermutationDefinitionInstance& permutationDefinition, ShaderPermutationId permutation, ShaderBuilder* shaderBuilder, ResultCode& errorOut)
{
    ShaderProgramDefinition definition;
    definition.pipelineType     = description.pipelineType;
    definition.intermediateCode = shaderBuilder->getIntermediateCode();
    const ShaderLanguage language   = description.language;

    std::vector<PreprocessDefine> preprocessDefines = { };
    preprocessDefines.resize(permutationDefinition.size());
    for (U32 i = 0; i < preprocessDefines.size(); ++i)
    {
        preprocessDefines[i].variable = permutationDefinition[i].name;
        preprocessDefines[i].value = std::to_string(makeBitset32(permutationDefinition[i].offset, permutationDefinition[i].size, permutationDefinition[i].value));
    }

    switch (definition.pipelineType)
    {
    case BindType_Compute:
        R_ASSERT_FORMAT(description.compute.cs, "Must have a valid compute shader, in order to build a ShaderProgram!");
        definition.compute.cs               = compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.compute.csName, description.compute.cs, permutation, language, ShaderType_Compute, preprocessDefines, errorOut);
        break;
    case BindType_Graphics:
        R_ASSERT_FORMAT(description.graphics.vs, "Must have at least a valid vertex shader, in order to build a ShaderProgram!");
        definition.graphics.usesMeshShaders = description.graphics.usesMeshShaders;
        if (description.graphics.usesMeshShaders)
        {
            definition.graphics.as          = description.graphics.as ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.asName, description.graphics.as, permutation, language, ShaderType_Amplification, preprocessDefines, errorOut) : nullptr;
            definition.graphics.ms          = compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.msName, description.graphics.ms, permutation, language, ShaderType_Mesh, preprocessDefines, errorOut);
        }
        else
        {
            definition.graphics.vs          = compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.vsName, description.graphics.vs, permutation, language, ShaderType_Vertex, preprocessDefines, errorOut);
            definition.graphics.gs          = description.graphics.gs ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.gsName, description.graphics.gs, permutation, language, ShaderType_Geometry, preprocessDefines, errorOut) : nullptr;
            definition.graphics.hs          = description.graphics.hs ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.hsName, description.graphics.hs, permutation, language, ShaderType_Hull, preprocessDefines, errorOut) : nullptr;
            definition.graphics.ds          = description.graphics.ds ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.dsName, description.graphics.ds, permutation, language, ShaderType_Domain, preprocessDefines, errorOut) : nullptr;
        }
        definition.graphics.ps              = description.graphics.ps ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.psName, description.graphics.ps, permutation, language, ShaderType_Pixel, preprocessDefines, errorOut) : nullptr;
        break;
    case BindType_RayTrace:
        definition.raytrace.rany            = description.raytrace.rany ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.ranyName, description.raytrace.rany, permutation, language, ShaderType_RayAnyHit, preprocessDefines, errorOut) : nullptr;
        definition.raytrace.rclosest        = description.raytrace.rclosest ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.rclosestName, description.raytrace.rclosest, permutation, language, ShaderType_RayClosestHit, preprocessDefines, errorOut) : nullptr;
        definition.raytrace.rgen            = description.raytrace.rgen ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.rgenName, description.raytrace.rgen, permutation, language, ShaderType_RayGeneration, preprocessDefines, errorOut) : nullptr;
        definition.raytrace.rintersect      = description.raytrace.rintersect ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.rintersectName, description.raytrace.rintersect, permutation, language, ShaderType_RayIntersect, preprocessDefines, errorOut) : nullptr;
        definition.raytrace.rmiss           = description.raytrace.rmiss ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.rmissName, description.raytrace.rmiss, permutation, language, ShaderType_RayMiss, preprocessDefines, errorOut) : nullptr;
        break;
    }

    if (errorOut != RecluseResult_Ok)
    {
        errorOut = RecluseResult_Failed;
    }
    else
    {
        // Work the shaders to create shader program reflection data.
        ShaderProgramReflection& programReflection = definition.programReflection;
        std::set<ShaderBind> cbvSet;
        std::set<ShaderBind> srvSet;
        std::set<ShaderBind> uavSet;
        std::set<ShaderBind> samplerSet;
        // Flip through each shader associated with this program, and determine the slots that it reflects.
        for (auto it : definition.shaderReflectionInfo)
        {
            ShaderReflection& reflection = it.second;
            for (U32 index = 0; index < reflection.cbvs.size(); ++index)
            {
                ShaderBind cbv = reflection.cbvs[index];
                auto result = cbvSet.insert(cbv);
                if (result.second)
                    programReflection.cbvs[index] = cbv;
            }
            for (U32 index = 0; index < reflection.srvs.size(); ++index)
            {
                ShaderBind srv = reflection.srvs[index];
                auto result = srvSet.insert(srv);
                if (result.second)
                    programReflection.srvs[index] = srv;
            }
            for (U32 index = 0; index < reflection.uavs.size(); ++index)
            {
                ShaderBind uav = reflection.uavs[index];
                auto result = uavSet.insert(uav);
                if (result.second)
                    programReflection.uavs[index] = uav;
            }
            for (U32 index = 0; index < reflection.samplers.size(); ++index)
            {
                ShaderBind sampler = reflection.samplers[index];
                auto result = samplerSet.insert(sampler);
                if (result.second)
                    programReflection.samplers[index] = sampler;
            }
        }
        programReflection.numCbvs = cbvSet.size();
        programReflection.numSrvs = srvSet.size();
        programReflection.numUavs = uavSet.size();
        programReflection.numSamplers = samplerSet.size();
    }

    return definition;
}


ResultCode buildShaderProgramDefinitions(ShaderProgramDatabase& db, const ShaderProgramDescription& description, ShaderProgramId outId, ShaderIntermediateCode imm)
{
    if (g_shaderBuilderMap.find(imm) == g_shaderBuilderMap.end())
    {
        g_shaderBuilderMap[imm] = createShaderBuilder(g_shaderBuilderName, imm);
        g_shaderBuilderMap[imm]->setUp();
    }

    ResultCode result                  = RecluseResult_Ok;
    ShaderBuilder* shaderBuilder    = g_shaderBuilderMap[imm];
    R_ASSERT(shaderBuilder != NULL);

    auto makeInstanceFunc = [&description, shaderBuilder, outId, &db] (const ShaderProgramPermutationDefinitionInstance& permutationDefinition, ShaderProgramPermutation permutation) -> ResultCode
    {
        ResultCode result = RecluseResult_Ok;
        ShaderProgramDefinition definition = makeShaderProgramDefinition(db, description, permutationDefinition, permutation, shaderBuilder, result);
        if (result != RecluseResult_Ok)
        {
            destroyShaderProgramDefinition(db, definition);
        }
        else
        {
            db.storeShaderProgramDefinition(definition, outId, permutation);
            
        }

        return result;
    };
 
    if (!db.hasShaderProgramDefinitions(outId))
    { 
        // Our first creation has no permutation. So we pass in an empty permutation.
        // 
        result = makeInstanceFunc(ShaderProgramPermutationDefinitionInstance(), 0);

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

                result = makeInstanceFunc(permutationDefinition, permutation);
                if (result != RecluseResult_Ok)
                {
                    break;
                }
            }
        }
    }
    return result;
}
} // 
} // 
} //