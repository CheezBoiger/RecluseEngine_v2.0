//
#include "Recluse/Graphics/ShaderProgram.hpp"


namespace Recluse {
namespace Pipeline {
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

R_PUBLIC_API ResultCode                     buildShaderProgramDefinitions(ShaderProgramDatabase& db, const ShaderProgramDescription& description, ShaderProgramId outId, ShaderIntermediateCode imm);
// Build the needed shader program definitions in order to define how the programs should be built in runtime.
R_PUBLIC_API ResultCode                     buildShaderProgram(const ShaderProgramDescription& description, ShaderProgramId outId, ShaderIntermediateCode imm);
R_PUBLIC_API ResultCode                     releaseShaderProgram(ShaderProgramId id);

// Function to allow saving program definitions on disk.
R_PUBLIC_API ResultCode                     saveProgramDefinitions(const std::string& dataPath);
R_PUBLIC_API ResultCode                     loadProgramDefinitions(const std::string& dataPath);
} // Builder
} // Pipeline
} // Recluse