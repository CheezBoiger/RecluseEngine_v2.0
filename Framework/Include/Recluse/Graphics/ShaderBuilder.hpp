//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"

#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {


// ShaderBuilder, handles high level shading languages, and transforms them into
// bytecode to be read to the gpu.
class R_EXPORT ShaderBuilder {
public:
    ShaderBuilder(ShaderIntermediateCode imm)
        : m_imm(imm) { }
    virtual ~ShaderBuilder() { }
    
    // Set up the shader builder. Should be the warm up setup.
    virtual ErrType setUp() { return REC_RESULT_NOT_IMPLEMENTED; }

    // Tear down the shader builder, that was initialized. Everything 
    // that is initialized by the shaderbuilder should be cleaned up.
    virtual ErrType tearDown() { return REC_RESULT_NOT_IMPLEMENTED; }

    // compiler the shader and return the bytecode.
    ErrType compile(Shader* pShader, const char* srcCode, 
        U64 sourceCodeBytes, ShaderLang lang, ShaderType shaderType);

    virtual ErrType disassemble(std::vector<char>& output) { return REC_RESULT_NOT_IMPLEMENTED; }

    ShaderIntermediateCode getIntermediateCode() const { return m_imm; }

private:

    virtual ErrType onCompile(const std::vector<char>& srcCode, std::vector<char>& byteCode, 
        ShaderLang lang, ShaderType shaderType) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    ShaderIntermediateCode m_imm;
};

// Must be newly allocated.
R_EXPORT ShaderBuilder* createGlslangShaderBuilder(ShaderIntermediateCode imm);
R_EXPORT ShaderBuilder* createDxcShaderBuilder(ShaderIntermediateCode imm);

static void freeShaderBuilder(ShaderBuilder* pBuilder) {
    delete pBuilder;
}
}