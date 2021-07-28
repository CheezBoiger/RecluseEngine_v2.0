//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"

#include "Graphics/ShaderMap.hpp"

#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {


enum ShaderTarget {
    
};


class ShaderBuilder {
public:
    ShaderBuilder(ShaderType shaderType, ShaderIntermediateCode imm)
        : m_shaderType(shaderType)
        , m_imm(imm) { }
    virtual ~ShaderBuilder() { }

    // compiler the shader and return the bytecode.
    virtual ErrType compile(const std::vector<char>& srcCode, std::vector<char>& byteCode, ShaderLang lang) {
        return REC_RESULT_NOT_IMPLEMENTED;
    }

    ShaderType getShaderType() const { return m_shaderType; }
    ShaderIntermediateCode getIntermediateCode() const { return m_imm; }
private:
    ShaderType m_shaderType;
    ShaderIntermediateCode m_imm;
};

// Must be newly allocated.
ShaderBuilder* createGlslangShaderBuilder(ShaderType shaderType, ShaderIntermediateCode imm);
ShaderBuilder* createDxcShaderBuilder(ShaderType shaderType, ShaderIntermediateCode imm);

static void freeShaderBuilder(ShaderBuilder* pBuilder) {
    delete pBuilder;
}
}