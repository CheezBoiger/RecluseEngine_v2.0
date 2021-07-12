// 
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


enum ShaderIntermediateCode {
    INTERMEDIATE_UNKNOWN,
    INTERMEDIATE_DXIL,
    INTERMEDIATE_DXBC,
    INTERMEDIATE_SPIRV
};


enum ShaderType {
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_HULL,
    SHADER_TYPE_TESS_CONTROL = SHADER_TYPE_HULL,
    SHADER_TYPE_DOMAIN,
    SHADER_TYPE_TESS_EVAL = SHADER_TYPE_DOMAIN,
    SHADER_TYPE_GEOMETRY,
    SHADER_TYPE_PIXEL,
    SHADER_TYPE_FRAGEMENT = SHADER_TYPE_PIXEL,
    SHADER_TYPE_RAY_GEN,
    SHADER_TYPE_RAY_CLOSESTHIT,
    SHADER_TYPE_RAY_ANYHIT,
    SHADER_TYPE_RAY_INTERSECT,
    SHADER_TYPE_RAY_MISS,
    SHADER_TYPE_AMPLIFICATION,
    SHADER_TYPE_MESH
};


class Shader {
public:
    virtual ~Shader() { }

    Shader(ShaderIntermediateCode intermediateCode, ShaderType type)
        : m_intermediateCode(intermediateCode)
        , m_shaderType(type) { }

    ShaderIntermediateCode getIntermediateCodeType() const { return m_intermediateCode; }

    ErrType compile(char* sourceCode, U32 sourceCodeBytes);

    U32 disassemble(char* disassembledCode);
    
    Shader convertTo(ShaderIntermediateCode intermediateCode);

    ShaderType getType() const { return m_shaderType; }

private:
    ShaderIntermediateCode m_intermediateCode;
    ShaderType m_shaderType;
};
} // Recluse