//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"

#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {


// ShaderBuilder, handles high level shading languages, and transforms them into
// bytecode to be read to the gpu.
class R_PUBLIC_API ShaderBuilder 
{
public:
    ShaderBuilder(ShaderIntermediateCode imm)
        : m_imm(imm) { }
    virtual ~ShaderBuilder() { }
    
    // Set up the shader builder. Should be the warm up setup.
    virtual ErrType setUp() { return R_RESULT_NO_IMPL; }

    // Tear down the shader builder, that was initialized. Everything 
    // that is initialized by the shaderbuilder should be cleaned up.
    virtual ErrType tearDown() { return R_RESULT_NO_IMPL; }

    // compiler the shader and return the bytecode.
    ErrType compile
        (
            Shader* pShader, 
            const char* srcCode, 
            U64 sourceCodeBytes, 
            ShaderLang lang, 
            ShaderType shaderType
        );

    virtual ErrType disassemble(std::vector<char>& output) { return R_RESULT_NO_IMPL; }

    ShaderIntermediateCode getIntermediateCode() const { return m_imm; }

private:

    virtual ErrType onCompile
                        (
                            const std::vector<char>& srcCode, 
                            std::vector<char>& byteCode, 
                            ShaderLang lang, 
                            ShaderType shaderType
                        ) 
        { return R_RESULT_NO_IMPL; }

    ShaderIntermediateCode m_imm;
};

// Must be newly allocated.
R_PUBLIC_API ShaderBuilder* createGlslangShaderBuilder(ShaderIntermediateCode imm);
R_PUBLIC_API ShaderBuilder* createDxcShaderBuilder(ShaderIntermediateCode imm);

// Must call when cleaning up our shader builders.
static void freeShaderBuilder(ShaderBuilder* pBuilder) 
{
    delete pBuilder;
}
}