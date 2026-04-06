//
#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"

#include "Recluse/Pipeline/Graphics/Reflection/SpirvReflection.hpp"
#include "Recluse/Pipeline/Graphics/Reflection/DxilReflection.hpp"

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include <unordered_map>
#include <unordered_set>
#include <set>

R_DECLARE_GLOBAL_STRING(g_shaderBuilderName, "glslang", "ShaderBuilder.NameId");

namespace Recluse {
namespace Pipeline {
namespace Builder {


//std::unordered_map<ShaderIntermediateCode, ShaderBuilder*> g_shaderBuilderMap;

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
        std::map<ShaderType, ShaderReflectionInformation>& reflectionOut,
        const char* entryPoint, 
        const std::string& shaderCode, 
        ShaderPermutationId permutation,
        ShaderLanguage language, 
        ShaderType shaderType,
        ShaderIntermediateCode intermediateCode,
        const std::vector<PreprocessDefine>& defines,
        ResultCode& errorOut,
        Bool debug = false
    )
{
    Shader* shader = nullptr;
    if (!shaderCode.empty())
    {
        ResultCode error = RecluseResult_Ok;
        Hash64 shaderHash = db.makeShaderHash(shaderCode.data(), sizeof(char) * shaderCode.size());
        if (!db.hasShader(shaderType, shaderHash))
        {
            shader = Shader::create();
            if (error == RecluseResult_Ok)
            { 
                ShaderBuilder::Config config = { };
                config.shaderType = shaderType;
                config.intermediateCode = intermediateCode;
                config.shaderLanguage = language;
                config.dumpSymbols = debug;
                config.stripDebugInfo = false; // Not supported for spirv yet.
                config.option = ShaderBuilder::Config::Default;

                error = shaderBuilder->compile(shader, entryPoint, shaderCode.data(), shaderCode.size(), config, nullptr, defines);
                if (error != RecluseResult_Ok)
                {
                    Shader::destroy(shader);
                    shader = nullptr;
                }
                else
                {
                    ShaderReflectionInformation reflectionData = { };
                    
                    ResultCode reflectError = RecluseResult_Ok;
                    switch (intermediateCode)
                    {
                        case ShaderIntermediateCode_Spirv:
                            {
                                SpirvReflection spirvReflection;
                                reflectError = spirvReflection.reflect(reflectionData, shader);
                                break;
                            }
                        case ShaderIntermediateCode_Dxil:
                            {
                                DxilReflection dxilReflection;
                                reflectError = dxilReflection.reflect(reflectionData, shader);
                                break;
                            }
                        default:
                            reflectError = RecluseResult_NotFound;
                            break;
                    }

                    if (reflectError == RecluseResult_Ok)
                    {
                        for (U32 i = 0; i < reflectionData.perSetMetadata.size(); ++i)
                        {
                            ShaderReflectionInformation::Metadata& metadata = reflectionData.perSetMetadata[i];
                            R_DEBUG("ShaderBuilder", "ShaderName: %s - Space %d \nCBVs: %d\nSRVs:%d\nUAVs:%d\nSamplers: %d", 
                                shader->getName(), i, metadata.numCbvs, metadata.numSrvs, 
                                metadata.numUavs, metadata.numSamplers);
                        }

                        reflectionOut.insert(std::make_pair(shaderType, reflectionData));
                    }
                }
            }
        }
        else
        {
            shader = db.obtainShader(shaderType, shaderHash);
        }
        errorOut |= error;
    }
    return shader;
}


R_INTERNAL 
ShaderProgramDefinition makeShaderProgramDefinition(ShaderProgramDatabase& db, const ShaderProgramDescription& description, const ShaderProgramPermutationDefinitionInstance& permutationDefinition, ShaderPermutationId permutation, ShaderBuilder* shaderBuilder, ShaderIntermediateCode intermediateCode, ResultCode& errorOut)
{
    ShaderProgramDefinition definition;
    definition.pipelineType     = description.pipelineType;
    definition.intermediateCode = intermediateCode;
    const ShaderLanguage language   = description.language;

    std::vector<PreprocessDefine> preprocessDefines = { };
    preprocessDefines.resize(permutationDefinition.size());
    for (U32 i = 0; i < preprocessDefines.size(); ++i)
    {
        preprocessDefines[i].variable = permutationDefinition[i].name;
        preprocessDefines[i].value = std::to_string(permutationDefinition[i].value);
    }

    switch (definition.pipelineType)
    {
    case BindType_Compute:
        R_ASSERT_FORMAT(description.compute.cs, "Must have a valid compute shader, in order to build a ShaderProgram!");
        definition.compute.cs               = compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.compute.csName, description.compute.cs, permutation, language, ShaderType_Compute, intermediateCode, preprocessDefines, errorOut);
        break;
    case BindType_Graphics:
        definition.graphics.usesMeshShaders = description.graphics.usesMeshShaders;
        if (description.graphics.usesMeshShaders)
        {
            definition.graphics.as          = description.graphics.as ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.asName, description.graphics.as, permutation, language, ShaderType_Amplification, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
            definition.graphics.ms          = compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.msName, description.graphics.ms, permutation, language, ShaderType_Mesh, intermediateCode, preprocessDefines, errorOut, description.debug);
        }
        else
        {
            R_ASSERT_FORMAT(description.graphics.vs, "Must have at least a valid vertex shader, in order to build a ShaderProgram!");
            definition.graphics.vs          = compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.vsName, description.graphics.vs, permutation, language, ShaderType_Vertex, intermediateCode, preprocessDefines, errorOut, description.debug);
            definition.graphics.gs          = description.graphics.gs ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.gsName, description.graphics.gs, permutation, language, ShaderType_Geometry, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
            definition.graphics.hs          = description.graphics.hs ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.hsName, description.graphics.hs, permutation, language, ShaderType_Hull, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
            definition.graphics.ds          = description.graphics.ds ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.dsName, description.graphics.ds, permutation, language, ShaderType_Domain, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
        }
        definition.graphics.ps              = description.graphics.ps ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.graphics.psName, description.graphics.ps, permutation, language, ShaderType_Pixel, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
        break;
    case BindType_RayTrace:
        definition.raytrace.rany            = description.raytrace.rany ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.ranyName, description.raytrace.rany, permutation, language, ShaderType_RayAnyHit, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
        definition.raytrace.rclosest        = description.raytrace.rclosest ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.rclosestName, description.raytrace.rclosest, permutation, language, ShaderType_RayClosestHit, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
        definition.raytrace.rgen            = description.raytrace.rgen ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.rgenName, description.raytrace.rgen, permutation, language, ShaderType_RayGeneration, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
        definition.raytrace.rintersect      = description.raytrace.rintersect ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.rintersectName, description.raytrace.rintersect, permutation, language, ShaderType_RayIntersect, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
        definition.raytrace.rmiss           = description.raytrace.rmiss ? compileShader(shaderBuilder, db, definition.shaderReflectionInfo, description.raytrace.rmissName, description.raytrace.rmiss, permutation, language, ShaderType_RayMiss, intermediateCode, preprocessDefines, errorOut, description.debug) : nullptr;
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
        struct ReflectionSet 
        {
            std::set<ShaderBind> cbvSet;
            std::set<ShaderBind> srvSet;
            std::set<ShaderBind> uavSet;
            std::set<ShaderBind> samplerSet;

            U8 baseCbv = 0xffff;
            U8 baseSrv = 0xffff;
            U8 baseUav = 0xffff;
            U8 baseSampler = 0xffff;
        };

        std::map<U32, ReflectionSet> reflectionSets;

        // Flip through each shader associated with this program, and determine the slots that it reflects.
        for (auto it : definition.shaderReflectionInfo)
        {
            ShaderReflectionInformation& reflection = it.second;
                
            for (U32 index = 0; index < reflection.cbvs.size(); ++index)
            {
                ShaderBind cbv = reflection.cbvs[index];
                U32 set = ShaderReflectionInformation::unpackShaderSet(cbv);
                U32 binding = ShaderReflectionInformation::unpackShaderBind(cbv);
                ShaderReflectionInformation::Metadata& metadata = reflection.perSetMetadata[set];
                if (reflectionSets[set].baseCbv < metadata.baseCbv)
                {
                    // Calculate append
                    binding = binding + metadata.baseCbv;
                    cbv = ShaderReflectionInformation::packShaderBinding(set, binding);
                }
                auto result = reflectionSets[set].cbvSet.insert(cbv);
                reflectionSets[set].baseCbv = Math::minimum(reflectionSets[set].baseCbv, metadata.baseCbv);
            }
            for (U32 index = 0; index < reflection.srvs.size(); ++index)
            {
                ShaderBind srv = reflection.srvs[index];
                U32 set = ShaderReflectionInformation::unpackShaderSet(srv);
                U32 binding = ShaderReflectionInformation::unpackShaderBind(srv);
                ShaderReflectionInformation::Metadata& metadata = reflection.perSetMetadata[set];
                if (reflectionSets[set].baseSrv < metadata.baseSrv)
                {
                    // Calculate append
                    binding = binding + metadata.baseSrv;
                    srv = ShaderReflectionInformation::packShaderBinding(set, binding);
                }
                auto result = reflectionSets[set].srvSet.insert(srv);
                reflectionSets[set].baseSrv = Math::minimum(reflectionSets[set].baseSrv, metadata.baseSrv);
            }
            for (U32 index = 0; index < reflection.uavs.size(); ++index)
            {
                ShaderBind uav = reflection.uavs[index];
                U32 set = ShaderReflectionInformation::unpackShaderSet(uav);
                U32 binding = ShaderReflectionInformation::unpackShaderBind(uav);
                ShaderReflectionInformation::Metadata& metadata = reflection.perSetMetadata[set];
                if (reflectionSets[set].baseUav < metadata.baseUav)
                {
                    // Calculate append
                    binding = binding + metadata.baseUav;
                    uav = ShaderReflectionInformation::packShaderBinding(set, binding);
                }
                auto result = reflectionSets[set].uavSet.insert(uav);
                reflectionSets[set].baseUav = Math::minimum(reflectionSets[set].baseUav, metadata.baseUav);
            }
            for (U32 index = 0; index < reflection.samplers.size(); ++index)
            {
                ShaderBind sampler = reflection.samplers[index];
                U32 set = ShaderReflectionInformation::unpackShaderSet(sampler);
                U32 binding = ShaderReflectionInformation::unpackShaderBind(sampler);
                ShaderReflectionInformation::Metadata& metadata = reflection.perSetMetadata[set];
                if (reflectionSets[set].baseSampler < metadata.baseSampler)
                {
                    // Calculate append
                    binding = binding + metadata.baseSampler;
                    sampler = ShaderReflectionInformation::packShaderBinding(set, binding);
                }
                auto result = reflectionSets[set].samplerSet.insert(sampler);
                reflectionSets[set].baseSampler = Math::minimum(reflectionSets[set].baseSampler, metadata.baseSampler);
            }
        }

        // After we have all the shader sets and bindings laid out, we store into the program reflection.
        programReflection.sets.resize(reflectionSets.size());

        for (const auto& set : reflectionSets)
        {
            u32 i = 0;
            U8 srvCount = 0;
            U8 cbvCount = 0;
            U8 uavCount = 0;
            U8 samplerCount = 0;

            for (ShaderBind shaderBind : set.second.cbvSet)
            {
                U32 space = ShaderReflectionInformation::unpackShaderSet(shaderBind);
                U32 bind = ShaderReflectionInformation::unpackShaderBind(shaderBind);
                programReflection.sets[space].cbvs[bind] = bind;
                cbvCount = Math::maximum<U8>(cbvCount, bind+1);
            }
            i = 0;
            for (ShaderBind shaderBind : set.second.srvSet)
            {
                U32 space = ShaderReflectionInformation::unpackShaderSet(shaderBind);
                U32 bind = ShaderReflectionInformation::unpackShaderBind(shaderBind);
                programReflection.sets[space].srvs[bind] = bind;
               srvCount = Math::maximum<U8>(srvCount, bind+1);
            }
            i = 0;
            for (ShaderBind shaderBind : set.second.uavSet)
            {
                U32 space = ShaderReflectionInformation::unpackShaderSet(shaderBind);
                U32 bind = ShaderReflectionInformation::unpackShaderBind(shaderBind);
                programReflection.sets[space].uavs[bind] = bind;
                uavCount = Math::maximum<U8>(uavCount, bind+1);
            }
            i = 0;
            for (ShaderBind shaderBind : set.second.samplerSet)
            {
                U32 space = ShaderReflectionInformation::unpackShaderSet(shaderBind);
                U32 bind = ShaderReflectionInformation::unpackShaderBind(shaderBind);
                programReflection.sets[space].samplers[bind] = bind;
                samplerCount = Math::maximum<U8>(samplerCount, bind+1);
            }
            programReflection.sets[set.first].numCbvs = cbvCount;
            programReflection.sets[set.first].numSrvs = srvCount;
            programReflection.sets[set.first].numUavs = uavCount;
            programReflection.sets[set.first].numSamplers = samplerCount;
            programReflection.sets[set.first].baseCbv = set.second.baseCbv;
            programReflection.sets[set.first].baseSrv = set.second.baseSrv;
            programReflection.sets[set.first].baseUav = set.second.baseUav;
            programReflection.sets[set.first].baseSampler = set.second.baseSampler;
        }
    }

    return definition;
}


ResultCode buildShaderProgram(ShaderProgramDatabase& db, const ShaderProgramDescription& description, ShaderProgramId outId, ShaderIntermediateCode intermediateCode, ShaderBuilder* shaderBuilder)
{
    ResultCode result                  = RecluseResult_Ok;
    R_ASSERT(shaderBuilder != NULL);

    auto makeInstanceFunc = [&description, shaderBuilder, outId, &db, intermediateCode] (const ShaderProgramPermutationDefinitionInstance& permutationDefinition, ShaderProgramPermutation permutation) -> ResultCode
    {
        ResultCode result = RecluseResult_Ok;
        ShaderProgramDefinition definition = makeShaderProgramDefinition(db, description, permutationDefinition, permutation, shaderBuilder, intermediateCode, result);
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


ResultCode buildShaderPrograms(ShaderProgramDatabase& db, const ShaderProgramDescriptionInfo* descriptions, ShaderIntermediateCode intermediateCode, ShaderBuilder* shaderBuilder)
{
    R_ASSERT(descriptions != NULL);
    R_ASSERT(descriptions->descriptions.size() == descriptions->shaderProgramIds.size());
    ResultCode result = RecluseResult_Ok;
    for (U32 i = 0; i < descriptions->descriptions.size(); ++i)
    {
        const ShaderProgramDescription& description = descriptions->descriptions[i];
        ShaderProgramId programId = descriptions->shaderProgramIds[i];
        result = buildShaderProgram(db, description, programId, intermediateCode, shaderBuilder);
        if (result != RecluseResult_Ok);
            break;
    }
    return result;
}


ShaderProgramDescription& ShaderProgramDescription::setPipelineType(BindType type)
{
    pipelineType = type;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setShaderLanguage(ShaderLanguage shaderLanguage)
{
    language = shaderLanguage;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setVertexShader(const char* vsSource, const char* vsEntry)
{
    R_ASSERT(vsSource != NULL && vsEntry != NULL);
    graphics.vs = vsSource;
    graphics.vsName = vsEntry;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setAmplificationShader(const char* asSource, const char* asEntry)
{
    R_ASSERT(asEntry != NULL);
    graphics.asName = asEntry;
    graphics.as = asSource;
    graphics.usesMeshShaders = true;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setTaskShader(const char* taskSource, const char* taskEntry)
{
    return setAmplificationShader(taskSource, taskEntry);
}


ShaderProgramDescription& ShaderProgramDescription::setMeshShader(const char* msSource, const char* msEntry)
{
    R_ASSERT(msEntry != NULL);
    graphics.msName = msEntry;
    graphics.ms = msSource;
    graphics.usesMeshShaders = true;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setPixelShader(const char* psSource, const char* psEntry)
{
    R_ASSERT(psEntry != NULL);
    graphics.psName = psEntry;
    graphics.ps = psSource;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setFragmentShader(const char* fsSource, const char* fsEntry)
{
    return setPixelShader(fsSource, fsEntry);
}


ShaderProgramDescription& ShaderProgramDescription::setComputeShader(const char* csSource, const char* csEntry)
{
    R_ASSERT(csEntry != NULL);
    R_ASSERT(csSource != NULL);
    compute.cs = csSource;
    compute.csName = csEntry;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setHullShader(const char* hsSource, const char* hsEntry)
{
    R_ASSERT(hsSource != NULL && hsEntry != NULL);
    graphics.hs = hsSource;
    graphics.hsName = hsEntry;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setDomainShader(const char* dsSource, const char* dsEntry)
{
    R_ASSERT(dsSource != NULL && dsEntry != NULL);
    graphics.ds = dsSource;
    graphics.dsName = dsEntry;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setRayGenShader(const char* rgSource, const char* rgEntry)
{
    R_ASSERT(rgSource != NULL && rgEntry != NULL);
    raytrace.rgen = rgEntry;
    raytrace.rgenName = rgEntry;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setRayMissShader(const char* rmSource, const char* rmEntry)
{
    R_ASSERT(rmSource != NULL && rmEntry != NULL);
    raytrace.rmiss = rmSource;
    raytrace.rmissName = rmEntry;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setRayAnyShader(const char* raSource, const char* raEntry)
{
    R_ASSERT(raSource != NULL && raEntry != NULL);
    raytrace.rany = raEntry;
    raytrace.ranyName = raSource;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setRayClosestShader(const char* rcSource, const char* rcEntry)
{
    R_ASSERT(rcSource != NULL && rcEntry != NULL);
    raytrace.rclosest = rcSource;
    raytrace.rclosestName = rcEntry;
    return (*this);
}


ShaderProgramDescription& ShaderProgramDescription::setRayIntersectShader(const char* riSource, const char* riEntry)
{
    R_ASSERT(riSource != NULL && riEntry != NULL);
    raytrace.rintersect = riSource;
    raytrace.rintersectName = riEntry;
    return (*this);
}
} // 
} // 
} //