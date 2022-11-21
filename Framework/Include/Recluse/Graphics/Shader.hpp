// 
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include <vector>

namespace Recluse {


enum ShaderIntermediateCode 
{
    ShaderIntermediateCode_Unknown,
    ShaderIntermediateCode_Dxil,
    ShaderIntermediateCode_Dxbc,
    ShaderIntermediateCode_Spirv
};


enum ShaderLang 
{
    ShaderLang_Glsl,
    ShaderLang_Hlsl,
    ShaderLang_Count
};


enum ShaderType 
{
    ShaderType_None                     = (0),
    ShaderType_Vertex                   = (1<<0),
    ShaderType_Hull                     = (1<<1),
    ShaderType_TessellationControl      = ShaderType_Hull,
    ShaderType_Domain                   = (1<<2),
    ShaderType_TessellationEvaluation   = ShaderType_Domain,
    ShaderType_Geometry                 = (1<<3),
    ShaderType_Pixel                    = (1<<4),
    ShaderType_Fragment                 = ShaderType_Pixel,
    ShaderType_RayGeneration            = (1<<5),
    ShaderType_RayClosestHit            = (1<<6),
    ShaderType_RayAnyHit                = (1<<7),
    ShaderType_RayIntersect             = (1<<8),
    ShaderType_RayMiss                  = (1<<9),
    ShaderType_Amplification            = (1<<10),
    ShaderType_Mesh                     = (1<<11),
    ShaderType_Compute                  = (1<<12),
    ShaderType_All                      = (0xFFFFFFFF)
};

typedef U32 ShaderTypeFlags;


// Shader is a container that uses the crc
class Shader final 
{
public:
    ~Shader() { }

    static R_PUBLIC_API Shader* create();
    static R_PUBLIC_API void destroy(Shader* pShader);

private:

    Shader()
        : m_intermediateCode(ShaderIntermediateCode_Unknown)
        , m_shaderType(ShaderType_None)
        , m_hashId(0ull) { }

public:

    ShaderIntermediateCode getIntermediateCodeType() const { return m_intermediateCode; }

    // Load the bytecode data to the shader. This requires that the data be in bytecode, not source!
    R_PUBLIC_API ErrType load(const char* byteCode, U64 szBytes, ShaderIntermediateCode imm, ShaderType shaderType);

    // Save the compilation to a file.
    R_PUBLIC_API ErrType saveToFile(const char* filePath);

    R_PUBLIC_API Shader* convertTo(ShaderIntermediateCode intermediateCode);

    ShaderType getType() const { return m_shaderType; }

    const char* getByteCode() const { return m_byteCode.data(); }
    
    U64 getSzBytes() const { return m_byteCode.size(); }

    Hash64 getHashId() const { return m_hashId; }

private:

    // Generate id from bytecode value.
    void genHashId();

    ShaderIntermediateCode  m_intermediateCode;
    ShaderType              m_shaderType;
    std::vector<char>       m_byteCode;
    Hash64                  m_hashId;
    
};
} // Recluse