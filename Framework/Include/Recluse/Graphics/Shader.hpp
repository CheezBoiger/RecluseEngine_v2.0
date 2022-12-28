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
    ShaderType_None,
    ShaderType_Vertex = 0,
    ShaderType_Hull,
    ShaderType_TessellationControl = ShaderType_Hull,
    ShaderType_Domain,
    ShaderType_TessellationEvaluation = ShaderType_Domain,
    ShaderType_Geometry,
    ShaderType_Pixel,
    ShaderType_Fragment = ShaderType_Pixel,
    ShaderType_RayGeneration,
    ShaderType_RayClosestHit,
    ShaderType_RayAnyHit,
    ShaderType_RayIntersect,
    ShaderType_RayMiss,
    ShaderType_Amplification,
    ShaderType_Mesh,
    ShaderType_Compute,
    ShaderType_Count
};

enum ShaderStage 
{
    ShaderStage_None                     = (ShaderType_None),
    ShaderStage_Vertex                   = (1<<ShaderType_Vertex),
    ShaderStage_Hull                     = (1<<ShaderType_Hull),
    ShaderStage_TessellationControl      = ShaderStage_Hull,
    ShaderStage_Domain                   = (1<<ShaderType_Domain),
    ShaderStage_TessellationEvaluation   = ShaderStage_Domain,
    ShaderStage_Geometry                 = (1<<ShaderType_Geometry),
    ShaderStage_Pixel                    = (1<<ShaderType_Pixel),
    ShaderStage_Fragment                 = ShaderStage_Pixel,
    ShaderStage_RayGeneration            = (1<<ShaderType_RayGeneration),
    ShaderStage_RayClosestHit            = (1<<ShaderType_RayClosestHit),
    ShaderStage_RayAnyHit                = (1<<ShaderType_RayAnyHit),
    ShaderStage_RayIntersect             = (1<<ShaderType_RayIntersect),
    ShaderStage_RayMiss                  = (1<<ShaderType_RayMiss),
    ShaderStage_Amplification            = (1<<ShaderType_Amplification),
    ShaderStage_Mesh                     = (1<<ShaderType_Mesh),
    ShaderStage_Compute                  = (1<<ShaderType_Compute),
    ShaderStage_All                      = (0xFFFFFFFF)
};

typedef U32 ShaderStageFlags;
typedef U32 ShaderId;

// Prevents having to write the actual bit manip.
static ShaderStageFlags shaderTypeToShaderStageFlags(ShaderType type)
{
    return (1<<type);
}

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
        , m_uniqueId(~0u)
        , m_entryPoint(nullptr) { }

public:

    ShaderIntermediateCode getIntermediateCodeType() const { return m_intermediateCode; }

    // Load the bytecode data to the shader. This requires that the data be in bytecode, not source!
    R_PUBLIC_API ErrType load(const char* entryPoint, const char* byteCode, U64 szBytes, ShaderIntermediateCode imm, ShaderType shaderType);

    // Save the compilation to a file.
    R_PUBLIC_API ErrType saveToFile(const char* filePath);

    R_PUBLIC_API Shader* convertTo(ShaderIntermediateCode intermediateCode);

    const char* getEntryPointName() const { return m_entryPoint; }

    ShaderType getType() const { return m_shaderType; }

    const char* getByteCode() const { return m_byteCode.data(); }
    
    U64 getSzBytes() const { return m_byteCode.size(); }

    ShaderId getId() const { return m_uniqueId; }

private:

    // Generate id from bytecode value.
    void genHashId();

    ShaderIntermediateCode  m_intermediateCode;
    ShaderType             m_shaderType;
    std::vector<char>       m_byteCode;
    ShaderId                m_uniqueId;
    const char*             m_entryPoint;
};
} // Recluse