// 
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/Utility.hpp"

#include <vector>

namespace Recluse {


enum ShaderIntermediateCode 
{
    ShaderIntermediateCode_Unknown,
    ShaderIntermediateCode_Dxil,
    ShaderIntermediateCode_Dxbc,
    ShaderIntermediateCode_Spirv
};


enum ShaderLanguage 
{
    ShaderLanguage_Glsl,
    ShaderLanguage_Hlsl,
    ShaderLanguage_Count
};


enum ShaderType
{
    ShaderType_None = -1,
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
    ShaderType_Task = ShaderType_Amplification,
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
    ShaderStage_Task                     = ShaderStage_Amplification,
    ShaderStage_Mesh                     = (1<<ShaderType_Mesh),
    ShaderStage_Compute                  = (1<<ShaderType_Compute),
    ShaderStage_All                      = (0xFFFFFFFF)
};

typedef U32 ShaderStageFlags;
typedef U64 ShaderId;

// Prevents having to write the actual bit manip.
static ShaderStageFlags shaderTypeToShaderStageFlags(ShaderType type)
{
    return (1<<type);
}


typedef U64 ShaderPermutationId;

// Shader is a container that uses the crc
class Shader final : public IReference, public Serializable
{
public:
    ~Shader() { }

    static R_PUBLIC_API Shader*     create();
    static R_PUBLIC_API void        destroy(Shader* pShader);
    static R_PUBLIC_API Hash64      makeShaderHash(const char* bytecode, U64 bytecodeLength);

private:

    Shader()
        : m_intermediateCode(ShaderIntermediateCode_Unknown)
        , m_shaderType(ShaderType_None)
        , m_shaderNameHash(~0u)
        , m_permutation(0)
        , m_instanceId(0)
        , m_shaderHashId(~0)
    {
        m_shaderName[0] = '\0'; 
        addReference();
    }

public:

    ShaderIntermediateCode  getIntermediateCodeType() const { return m_intermediateCode; }

    // Load the bytecode data to the shader. This requires that the data be in bytecode, not source!
    R_PUBLIC_API ResultCode load(const char* entryPoint, const char* byteCode, U64 szBytes, ShaderIntermediateCode imm, ShaderType shaderType);

    // Save the compilation to a file.
    R_PUBLIC_API ResultCode saveToFile(const char* filePath);

    R_PUBLIC_API Shader*    convertTo(ShaderIntermediateCode intermediateCode);
    const char*             getEntryPointName() const { return m_entryPoint.c_str(); }
    ShaderType              getType() const { return m_shaderType; }
    const char*             getByteCode() const { return m_byteCode.data(); }
    U64                     getSzBytes() const { return m_byteCode.size(); }
    ShaderId                getNameHashId() const { return m_shaderNameHash; }
    ShaderId                getInstanceId() const { return m_instanceId; }
    
    void                    setName(const char* name) 
    {
        m_shaderName = name;
        m_shaderNameHash = recluseHashFast(m_shaderName.data(), m_shaderName.size());
    }

    // Obtain the shader hash id, this is usually equal across the same shader with different permutations.
    // PermutationId defines the unique id between similar shaders, in order to distinguish from other combinations.
    Hash64                  getShaderHashId() const { return m_shaderHashId; }

    const char*             getName() const { return m_shaderName.c_str(); }
    U64                     getNameSize() const { return static_cast<U64>(m_shaderName.size()); }

    void                    setPermutationId(ShaderPermutationId permutation) { m_permutation = permutation; }
    ShaderPermutationId     getPermutationId() const { return m_permutation; }

    R_PUBLIC_API ResultCode serialize(Archive* archive) const override;
    R_PUBLIC_API ResultCode deserialize(Archive* archive) override;

private:

    ShaderIntermediateCode  m_intermediateCode;
    ShaderType              m_shaderType;
    std::vector<char>       m_byteCode;
    ShaderId                m_shaderNameHash;
    ShaderId                m_instanceId;
    Hash64                  m_shaderHashId;
    std::string             m_entryPoint;
    std::string             m_shaderName;
    ShaderPermutationId     m_permutation;
};


typedef U8 ReflectionBind;

// Shader Reflection information.
class ShaderReflection : public Serializable
{
public:
    struct 
    {
        U8  numCbvs;
        U8  numSrvs;
        U8  numUavs;
        U8  numSamplers;
        U8  numInputParameters;
        U8  numOutputParameters;
        U16 pad0;
    } metadata;
    std::vector<ReflectionBind> cbvs;
    std::vector<ReflectionBind> srvs;
    std::vector<ReflectionBind> uavs;
    std::vector<ReflectionBind> samplers;
    R_PUBLIC_API ResultCode serialize(Archive* archive) const override;
    R_PUBLIC_API ResultCode deserialize(Archive* archive) override;
};
} // Recluse