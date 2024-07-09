//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Serialization/Serializable.hpp"

#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/PipelineState.hpp"

#include <unordered_map>
#include <map>
#include <bitset>
#include <array>

namespace Recluse {

typedef Hash64 ShaderProgramId;
typedef U64 ShaderProgramPermutation;
class GraphicsDevice;


struct R_PUBLIC_API ShaderProgramReflection : public Serializable
{
    std::array<ShaderBind, 16>  cbvs;
    std::array<ShaderBind, 64>  srvs;
    std::array<ShaderBind, 8>   uavs;
    std::array<ShaderBind, 16>  samplers;
    union
    {
        struct
        {
            U8 numCbvs;
            U8 numSrvs;
            U8 numUavs;
            U8 numSamplers;
        };
        U32 packed32;
    };
    ShaderProgramReflection()
    {
        memset(cbvs.data(), (ShaderBind)-1, sizeof(ShaderBind) * cbvs.size());
        memset(srvs.data(), (ShaderBind)-1, sizeof(ShaderBind) * srvs.size());
        memset(uavs.data(), (ShaderBind)-1, sizeof(ShaderBind) * uavs.size());
        memset(samplers.data(), (ShaderBind)-1, sizeof(ShaderBind) * samplers.size());
        numCbvs = numUavs = numSrvs = numSamplers = 0;
    }

    ResultCode serialize(Archive* archive) const override;
    ResultCode deserialize(Archive* archive) override;
};


// Definition of a shader program. This is one instance of a program (with any necessary permutations set.)
// Keep in mind this is a definition struct, that holds onto the shader intermediate code of the rendering application. Since it is a reference,
// Once you load it into the graphics device, you can destroy this definition.
class R_PUBLIC_API ShaderProgramDefinition
{
public:
    // The pipeline that must be bound to the rendering backend.
    BindType pipelineType;
    // The intermediate code that is used by this definition.
    ShaderIntermediateCode intermediateCode;
    union 
    {
        struct 
        {
            union
            {
                struct
                {
                    Shader* vs; //< Vertex Shader
                    Shader* gs; //< Geometry Shader
                    Shader* hs; //< Hull Shader
                    Shader* ds; //< Domain Shader
                };
                struct 
                {
                    Shader* as; //< Amplification Shader
                    Shader* ms; //< Mesh Shader
                };
            };
            Shader* ps; //< Pixel Shader
            Bool    usesMeshShaders; // If we are using either traditional pipeline, or mesh pipeline.
        } graphics;
        struct 
        {
            Shader* cs; //< Compute Shader
        } compute;
        struct 
        {
            Shader* rgen; //< Ray Generation Shader
            Shader* rmiss; //< Ray Miss Shader
            Shader* rany; //< Ray Any Hit Shader
            Shader* rclosest; //< Ray Closest Hit Shader
            Shader* rintersect; //< Ray Intersect Shader
        } raytrace;
    };
    // Any Reflection Information that is used by each shader.
    std::map<ShaderType, ShaderReflection>  shaderReflectionInfo;
    ShaderProgramReflection                 programReflection;

    // Constructors. Any containers need to be initialized here.
    ShaderProgramDefinition() { }
    ShaderProgramDefinition(const ShaderProgramDefinition& def);
    ShaderProgramDefinition(ShaderProgramDefinition&& def);
    
    ShaderProgramDefinition& operator=(const ShaderProgramDefinition& def);
    ShaderProgramDefinition& operator=(ShaderProgramDefinition&& def);

    ~ShaderProgramDefinition() { }
};


class R_PUBLIC_API ShaderProgramDatabase : public Serializable
{
public:
    static Hash64                               makeShaderHash(const char* bytecode, U32 lengthBytes);
    typedef std::unordered_map<ShaderProgramPermutation, ShaderProgramDefinition> PermutationMap;
    typedef std::unordered_map<ShaderProgramId, PermutationMap> MetaMap;

    ShaderProgramDatabase(const std::string& dbname = "") 
        : m_name(dbname) 
    {
        m_nameHash = recluseHashFast(m_name.data(), m_name.size() * sizeof(char)); 
    }

    ~ShaderProgramDatabase() { }

    // Clear up shader program definitions to save space, if they are no longer needed.
    void                                        clearShaderProgramDefinitions();
    ShaderProgramDefinition*                    obtainShaderProgramDefinition(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);  
    ResultCode                                  releaseShaderProgramDefinition(ShaderProgramId program, ShaderProgramPermutation permutation);

    ResultCode                                  serialize(Archive* pArchive) const override;
    ResultCode                                  deserialize(Archive* pArchive) override;

    const MetaMap&                              obtainMetaMap() const { return m_shaderProgramMetaMap; }                        
    Bool                                        hasShader(Hash64 shaderHash) const
    {
        auto& it = m_shaderMap.find(shaderHash);
        return (it != m_shaderMap.end());  
    }

    Bool hasShaderProgramDefinitions(ShaderProgramId shaderProgram)
    {
        auto& it = m_shaderProgramMetaMap.find(shaderProgram);
        return (it != m_shaderProgramMetaMap.end());
    }

    Shader*                         obtainShader(Hash64 id) const;

    const PermutationMap* obtainShaderProgramPermutations(ShaderProgramId shaderProgram) const
    {
        auto iter = m_shaderProgramMetaMap.find(shaderProgram);
        if (iter != m_shaderProgramMetaMap.end())
            return &iter->second;
        return nullptr;
    }

    void                                        storeShaderProgramDefinition(const ShaderProgramDefinition& definition, ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);

private:
    void                                        storeShader(Shader* pShader);
    void                                        cleanUpShaderProgramDefinition(const ShaderProgramDefinition& definition);
    void                                        removeShader(Shader* pShader);

    std::map<Hash64, Shader*>                   m_shaderMap;
    MetaMap                                     m_shaderProgramMetaMap;
    std::string                                 m_name;
    Hash64                                      m_nameHash;
};


namespace Runtime {
// Build the actual shader program. This is the realtime runner that requires building the shaders and their respective permutations.
R_PUBLIC_API ResultCode                     buildShaderProgram(GraphicsDevice* pDevice, const ShaderProgramDatabase& db, ShaderProgramId shaderProgram);
R_PUBLIC_API ResultCode                     buildAllShaderPrograms(GraphicsDevice* pDevice, const ShaderProgramDatabase& db);
R_PUBLIC_API ResultCode                     releaseShaderProgram(GraphicsDevice* pDevice, ShaderProgramId shaderProgram);
R_PUBLIC_API ResultCode                     releaseAllShaderPrograms(GraphicsDevice* pDevice);
// Build all vertex input layouts, stores in their own register.
R_PUBLIC_API ResultCode                     buildVertexInputLayout(GraphicsDevice* pDevice, const VertexInputLayout& layout, VertexInputLayoutId inputLayoutId);
R_PUBLIC_API ResultCode                     releaseVertexInputLayout(GraphicsDevice* pDevice, VertexInputLayoutId inputLayoutId);
R_PUBLIC_API ResultCode                     releaseAllVertexInputLayouts(GraphicsDevice* pDevice);
} // Runtime
} // Recluse