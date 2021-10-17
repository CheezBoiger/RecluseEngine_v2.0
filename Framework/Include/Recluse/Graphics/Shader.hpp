// 
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include <vector>

namespace Recluse {


enum ShaderIntermediateCode {
    INTERMEDIATE_UNKNOWN,
    INTERMEDIATE_DXIL,
    INTERMEDIATE_DXBC,
    INTERMEDIATE_SPIRV
};


enum ShaderLang {
    SHADER_LANG_GLSL,
    SHADER_LANG_HLSL,
    SHADER_LANG_COUNT
};


enum ShaderType {
    SHADER_TYPE_NONE            = (0),
    SHADER_TYPE_VERTEX          = (1<<0),
    SHADER_TYPE_HULL            = (1<<1),
    SHADER_TYPE_TESS_CONTROL    = SHADER_TYPE_HULL,
    SHADER_TYPE_DOMAIN          = (1<<2),
    SHADER_TYPE_TESS_EVAL       = SHADER_TYPE_DOMAIN,
    SHADER_TYPE_GEOMETRY        = (1<<3),
    SHADER_TYPE_PIXEL           = (1<<4),
    SHADER_TYPE_FRAGMENT        = SHADER_TYPE_PIXEL,
    SHADER_TYPE_RAY_GEN         = (1<<5),
    SHADER_TYPE_RAY_CLOSESTHIT  = (1<<6),
    SHADER_TYPE_RAY_ANYHIT      = (1<<7),
    SHADER_TYPE_RAY_INTERSECT   = (1<<8),
    SHADER_TYPE_RAY_MISS        = (1<<9),
    SHADER_TYPE_AMPLIFICATION   = (1<<10),
    SHADER_TYPE_MESH            = (1<<11),
    SHADER_TYPE_COMPUTE         = (1<<12),
    SHADER_TYPE_ALL             = (0xFFFFFFFF)
};

typedef U32 ShaderTypeFlags;


class Shader final {
public:
    ~Shader() { }

    static R_PUBLIC_API Shader* create();
    static R_PUBLIC_API void destroy(Shader* pShader);

private:

    Shader()
        : m_intermediateCode(INTERMEDIATE_UNKNOWN)
        , m_shaderType(SHADER_TYPE_NONE)
        , m_crc(0ull) { }

public:

    ShaderIntermediateCode getIntermediateCodeType() const { return m_intermediateCode; }

    R_PUBLIC_API ErrType load(const char* byteCode, U64 szBytes, ShaderIntermediateCode imm, ShaderType shaderType);

    // Save the compilation to a file.
    R_PUBLIC_API ErrType saveToFile(const char* filePath);

    R_PUBLIC_API Shader* convertTo(ShaderIntermediateCode intermediateCode);

    ShaderType getType() const { return m_shaderType; }

    const char* getByteCode() const { return m_byteCode.data(); }
    
    U64 getSzBytes() const { return m_byteCode.size(); }

    Hash64 getCrc() const { return m_crc; }

private:

    // Generate CrC from bytecode value.
    void genCrc();

    ShaderIntermediateCode  m_intermediateCode;
    ShaderType              m_shaderType;
    std::vector<char>       m_byteCode;
    Hash64                  m_crc;
    
};
} // Recluse