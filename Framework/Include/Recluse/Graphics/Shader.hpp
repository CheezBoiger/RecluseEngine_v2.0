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
    SHADER_TYPE_VERTEX = (1<<0),
    SHADER_TYPE_HULL = (1<<1),
    SHADER_TYPE_TESS_CONTROL = SHADER_TYPE_HULL,
    SHADER_TYPE_DOMAIN = (1<<2),
    SHADER_TYPE_TESS_EVAL = SHADER_TYPE_DOMAIN,
    SHADER_TYPE_GEOMETRY = (1<<3),
    SHADER_TYPE_PIXEL = (1<<4),
    SHADER_TYPE_FRAGEMENT = SHADER_TYPE_PIXEL,
    SHADER_TYPE_RAY_GEN = (1<<5),
    SHADER_TYPE_RAY_CLOSESTHIT = (1<<6),
    SHADER_TYPE_RAY_ANYHIT = (1<<7),
    SHADER_TYPE_RAY_INTERSECT = (1<<8),
    SHADER_TYPE_RAY_MISS = (1<<9),
    SHADER_TYPE_AMPLIFICATION = (1<<10),
    SHADER_TYPE_MESH = (1<<11),
    SHADER_TYPE_ALL = (0xFFFFFFFF)
};

typedef U32 ShaderTypeFlags;


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