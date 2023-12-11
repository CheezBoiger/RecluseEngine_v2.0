//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"

#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {
namespace Pipeline {

struct PreprocessDefine
{
    std::string variable;
    std::string value;
};

// ShaderBuilder, handles high level shading languages, and transforms them into
// bytecode to be read to the gpu.
class R_PUBLIC_API ShaderBuilder 
{
public:
    ShaderBuilder(ShaderIntermediateCode imm)
        : m_imm(imm) { }
    virtual ~ShaderBuilder() { }
    
    // Set up the shader builder. Should be the warm up setup.
    virtual ResultCode setUp() { return RecluseResult_NoImpl; }

    // Tear down the shader builder, that was initialized. Everything 
    // that is initialized by the shaderbuilder should be cleaned up.
    virtual ResultCode tearDown() { return RecluseResult_NoImpl; }

    // compiler the shader and return the bytecode.
    ResultCode compile
        (
            Shader* pShader,
            const char* entryPoint,
            const char* srcCode, 
            U64 sourceCodeBytes, 
            ShaderPermutationId permutation,
            ShaderLanguage lang, 
            ShaderType shaderType,
            const std::vector<PreprocessDefine>& defines = std::vector<PreprocessDefine>()
        );

    virtual ResultCode disassemble(std::vector<char>& output) { return RecluseResult_NoImpl; }

    ShaderIntermediateCode getIntermediateCode() const { return m_imm; }

    // Shader reflection function.
    virtual ResultCode reflect(ShaderReflection& reflectionOutput, const char* bytecode, U64 sizeBytes, ShaderLanguage lang) { return { }; }

private:

    virtual ResultCode onCompile
                        (
                            const std::vector<char>& srcCode,
                            std::vector<char>& byteCode, 
                            const char* entryPoint,
                            ShaderLanguage lang, 
                            ShaderType shaderType,
                            const std::vector<PreprocessDefine>& defines = std::vector<PreprocessDefine>()
                        ) 
        { return RecluseResult_NoImpl; }

    virtual ResultCode preprocessInputResources(ShaderLanguage lang, std::vector<char>& sourceCode);

    ShaderIntermediateCode m_imm;
};

// Must be newly allocated.
R_PUBLIC_API ShaderBuilder* createShaderBuilder(const std::string& nameID, ShaderIntermediateCode intermediateCode);

// Must call when cleaning up our shader builders.
static void freeShaderBuilder(ShaderBuilder* pBuilder) 
{
    delete pBuilder;
}
} // Pipeline
} // Recluse