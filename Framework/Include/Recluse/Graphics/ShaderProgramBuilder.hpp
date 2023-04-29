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

namespace Builder {


struct R_PUBLIC_API ShaderProgramPermutationDefinition
{
    ShaderProgramPermutationDefinition(const char* name, U32 offset, U32 size, U32 value)
        : name(name), offset(offset), size(size), value(value) { }
    const char* name;
    U32 offset;
    U32 size;
    U32 value;
};

typedef std::vector<ShaderProgramPermutationDefinition> ShaderProgramPermutationDefinitionInstance;

struct R_PUBLIC_API ShaderProgramDescription
{
    union 
    {
        struct 
        {
            const char* ps;
            const char* psName;
            const char* vs;
            const char* vsName;
            const char* gs;
            const char* gsName;
            union 
            {
                struct 
                {
                    const char* hs;
                    const char* hsName;
                    const char* ds;
                    const char* dsName;
                };
                struct 
                {
                    const char* tessc;
                    const char* tesscName;
                    const char* tesse;
                    const char* tesseName;
                };
            };
        } graphics;
        struct 
        {
            const char* cs;
            const char* csName;
        } compute;
        struct {
            const char* rgen;
            const char* rgenName;
            const char* rmiss;
            const char* rmissName;
            const char* rany;
            const char* ranyName;
            const char* rclosest;
            const char* rclosestName;
            const char* rintersect;
            const char* rintersectName;
        } raytrace;
    };
    // Each instance in this vector holds contents that define a permutation.
    std::vector<ShaderProgramPermutationDefinitionInstance> permutationDefinitions;
    ShaderLang language;
    BindType pipelineType;

    ShaderProgramDescription() { memset(this, 0, sizeof(graphics)); }
    ShaderProgramDescription(const ShaderProgramDescription& description);
    ShaderProgramDescription(ShaderProgramDescription&& description);
    ~ShaderProgramDescription() { }
};

class R_PUBLIC_API ShaderProgramDefinition
{
public:
    BindType pipelineType;
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

    ~ShaderProgramDefinition() { }
};

// Build the needed shader program definitions in order to define how the programs should be built in runtime.
R_PUBLIC_API ResultCode                     buildShaderProgramDefinitions(const ShaderProgramDescription& description, ShaderProgramId outId, ShaderIntermediateCode imm);
R_PUBLIC_API ResultCode                     releaseShaderProgramDefinition(ShaderProgramId id);

// Function to allow saving program definitions on disk.
R_PUBLIC_API ResultCode                     saveProgramDefinitions(const std::string& dataPath);
R_PUBLIC_API ResultCode                     loadProgramDefinitions(const std::string& dataPath);

// Clear up shader program definitions to save space, if they are no longer needed.
R_PUBLIC_API void                        clearShaderProgramDefinitions();
R_PUBLIC_API ShaderProgramDefinition*    obtainShaderProgramDefinition(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);


namespace Runtime {
// Build the actual shader program. This is the realtime runner that requires building the shaders and their respective permutations.
//
R_PUBLIC_API ResultCode                     buildShaderProgram(GraphicsDevice* pDevice, ShaderProgramId shaderProgram);
R_PUBLIC_API ResultCode                     buildAllShaderPrograms(GraphicsDevice* pDevice);
R_PUBLIC_API ResultCode                     releaseShaderProgram(GraphicsDevice* pDevice, ShaderProgramId shaderProgram);
R_PUBLIC_API ResultCode                     releaseAllShaderPrograms(GraphicsDevice* pDevice);
// Build all vertex input layouts, stores in their own register.
R_PUBLIC_API ResultCode                     buildVertexInputLayout(GraphicsDevice* pDevice, const VertexInputLayout& layout, VertexInputLayoutId inputLayoutId);
R_PUBLIC_API ResultCode                     releaseVertexInputLayout(GraphicsDevice* pDevice, VertexInputLayoutId inputLayoutId);
R_PUBLIC_API ResultCode                     releaseAllVertexInputLayouts(GraphicsDevice* pDevice);
} // Runtime
} // Builder
} // Recluse