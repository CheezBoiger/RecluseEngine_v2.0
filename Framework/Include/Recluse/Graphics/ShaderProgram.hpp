//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"

#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/PipelineState.hpp"
#include <unordered_map>
#include <bitset>

namespace Recluse {

typedef Hash64 ShaderProgramId;
typedef U64 ShaderProgramPermutation;
class GraphicsDevice;



class R_PUBLIC_API ShaderProgramDefinition
{
public:
    BindType pipelineType;
    ShaderIntermediateCode intermediateCode;
    union 
    {
        struct 
        {
            Shader* vs;
            Shader* ps;
            Shader* gs;
            Shader* hs;
            Shader* ds;
        } graphics;
        struct 
        {
            Shader* cs;
        } compute;
        struct 
        {
            Shader* rgen;
            Shader* rmiss;
            Shader* rany;
            Shader* rclosest;
            Shader* rintersect;
        } raytrace;
    }; 

    ShaderProgramDefinition() { memset(this, 0, sizeof(ShaderProgramDefinition)); }
    ShaderProgramDefinition(const ShaderProgramDefinition& def);
    ShaderProgramDefinition(ShaderProgramDefinition&& def);
    
    ShaderProgramDefinition& operator=(const ShaderProgramDefinition& def);
    ShaderProgramDefinition& operator=(ShaderProgramDefinition&& def);

    ~ShaderProgramDefinition() { }
};


class R_PUBLIC_API ShaderProgramDatabase
{
public:
    static Hash64                               makeShaderHash(const char* bytecode, U32 lengthBytes, ShaderProgramPermutation permutation);
    typedef std::unordered_map<ShaderProgramPermutation, ShaderProgramDefinition> PermutationMap;
    typedef std::unordered_map<ShaderProgramId, PermutationMap> MetaMap;

    ShaderProgramDatabase() { }
    ~ShaderProgramDatabase() { }

    // Clear up shader program definitions to save space, if they are no longer needed.
    void                                        clearShaderProgramDefinitions();
    ShaderProgramDefinition*                    obtainShaderProgramDefinition(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);  
    ResultCode                                  releaseShaderProgramDefinition(ShaderProgramId program, ShaderProgramPermutation permutation);
    const MetaMap&                              obtainMetaMap() const { return g_shaderProgramMetaMap; }                        
    Bool                                        hasShader(Hash64 shaderHash) const
    {
        auto& it = g_shaderMap.find(shaderHash);
        return (it != g_shaderMap.end());  
    }

    Bool hasShaderProgramDefinitions(ShaderProgramId shaderProgram)
    {
        auto& it = g_shaderProgramMetaMap.find(shaderProgram);
        return (it != g_shaderProgramMetaMap.end());
    }

    Shader*                         obtainShader(Hash64 id) const;

    const PermutationMap* obtainShaderProgramPermutations(ShaderProgramId shaderProgram) const
    {
        auto iter = g_shaderProgramMetaMap.find(shaderProgram);
        if (iter != g_shaderProgramMetaMap.end())
            return &iter->second;
        return nullptr;
    }

    void                                        storeShaderProgramDefinition(const ShaderProgramDefinition& definition, ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);

private:
    void                                        storeShader(Shader* pShader);
    void                                        cleanUpShaderProgramDefinition(const ShaderProgramDefinition& definition);
    void                                        removeShader(Shader* pShader);

    std::map<Hash64, Shader*> g_shaderMap;
    MetaMap                                     g_shaderProgramMetaMap;
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