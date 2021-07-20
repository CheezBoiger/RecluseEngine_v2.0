//
#include "Recluse/Graphics/Shader.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include "Recluse/Messaging.hpp"

#if RCL_GLSLANG
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/Types.h"
#include "glslang/SPIRV/GlslangToSpv.h" 
#include "glslang/Include/ResourceLimits.h"

const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};
#endif

namespace Recluse {


Shader* Shader::create(ShaderIntermediateCode code, ShaderType type)
{
    return new Shader(code, type);
}


void Shader::destroy(Shader* pShader)
{
    if (pShader) {
        delete pShader;
    }
}


ErrType Shader::load(char* pByteCode, U64 szBytes)
{
    Hash64 hash = recluseHash(pByteCode, szBytes);
    m_crc = hash;
    m_byteCode.resize(szBytes);
    memcpy(m_byteCode.data(), pByteCode, szBytes);

    return REC_RESULT_OK;
}


ErrType Shader::compile(char* sourceCode, U64 sourceCodeBytes)
{
#if RCL_GLSLANG
    glslang::InitializeProcess();
    std::vector<char> srcCodeString;
    EShLanguage language = EShLangVertex;

    srcCodeString.resize(sourceCodeBytes + 1);

    memcpy(srcCodeString.data(), sourceCode, sourceCodeBytes);
    srcCodeString[sourceCodeBytes] = '\0';

    switch (m_shaderType) {
        case SHADER_TYPE_VERTEX: language = EShLangVertex; break;
        case SHADER_TYPE_FRAGEMENT: language = EShLangFragment; break;
        case SHADER_TYPE_COMPUTE: language = EShLangCompute; break;
        // TODO: Support more shaders!
    }

    glslang::TProgram program;
    glslang::TShader shader(language);
    std::vector<U32> spirv;
    spv::SpvBuildLogger logger;
    std::string compileLog;
    const char* str = srcCodeString.data();

    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    shader.setStrings(&str, 1);
    shader.setEntryPoint("main");
    shader.setAutoMapLocations(true);   // --aml, which will allow for implicit location mapping 
                                        // for builtin blocks.

    EShMessages messages = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);
    
    if (!shader.parse(&DefaultTBuiltInResource, 450, false, messages)) {

        R_ERR("Shader", "Compile Error: %s", shader.getInfoLog());

        return REC_RESULT_FAILED;

    }

    program.addShader(&shader);
    
    if (!program.link(messages)) {

        R_ERR("Shader", "Failed to link shader program!");

        return REC_RESULT_FAILED;

    }

    glslang::GlslangToSpv(*program.getIntermediate(language), spirv, &logger);
    m_byteCode.resize(spirv.size() * sizeof(U32)); // Byte code is in uint32 format.
    memcpy(m_byteCode.data(), spirv.data(), m_byteCode.size());
    
    glslang::FinalizeProcess();
    return REC_RESULT_OK;
#else
    return REC_RESULT_NOT_IMPLEMENTED;
#endif
}


Shader* Shader::convertTo(ShaderIntermediateCode intermediateCode)
{
    return nullptr;
}


U32 Shader::disassemble(char* disassembledCode)
{
    return 0;
}
} // Recluse