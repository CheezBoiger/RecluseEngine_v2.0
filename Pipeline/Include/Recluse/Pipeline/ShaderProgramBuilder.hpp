//
#pragma once
#include "Recluse/Graphics/ShaderProgram.hpp"


namespace Recluse {
namespace Pipeline {
namespace Builder {

// Shader program permutation definition, defines possible permutations for a given shader program.
struct R_PUBLIC_API ShaderProgramPermutationDefinition
{
    ShaderProgramPermutationDefinition(const char* name = nullptr, U32 offset = 0, U32 size = 0, U32 value = 0)
        : name(name), offset(offset), size(size), value(value) { }
    const char* name;
    U32 offset;
    U32 size;
    U32 value;
};

typedef std::vector<ShaderProgramPermutationDefinition> ShaderProgramPermutationDefinitionInstance;

// Shader Program description is a metadata struct that describes the shader program that 
// is to be created for rendering use. This essentially sets up the program that the backend renderer
// will use when binding to the graphics pipeline.
struct R_PUBLIC_API ShaderProgramDescription
{
    union 
    {
        struct 
        {
            const char* ps;                         // Pixel Shader source code.
            const char* psName;                     // Pixel Shader Function Entry Point name.
            union
            {
                struct 
                {
                    const char* vs;                 // Vertex Shader source code.
                    const char* vsName;             // Vertex Shader Function Entry Point name.
                    const char* gs;                 // Geometry Shader source code.
                    const char* gsName;             // Geometry Sahder Function Entry Point name.
                    union 
                    {
                        struct 
                        {
                            const char* hs;         // Hull Shader source code.
                            const char* hsName;     // Hull Shader Function Entry Point name.
                            const char* ds;         // Domain Shader source code.
                            const char* dsName;     // Domain Shader entry point name.
                        };
                        struct 
                        {
                            const char* tessc;      // Tessellation Control Shader source code.
                            const char* tesscName;  // Tessellation Control Shader entry point name.
                            const char* tesse;      // Tessellation Evaluation Shader source code.
                            const char* tesseName;  // Tessellation Evaluation Shader entry point name.
                        };
                    };
                };
                struct
                {
                    const char* as;                 // Amplification Shader source code.
                    const char* asName;             // Amplification Shader entry point name.
                    const char* ms;                 // Mesh Shader source code.
                    const char* msName;             // Mesh Shader entry point name.
                };              
            };
            Bool usesMeshShaders;                   // Checks if we use mesh shader pipeline.
        } graphics;
        struct 
        {
            const char* cs;                         // Compute Shader source code.
            const char* csName;                     // Compute Shader entry point name.
        } compute;
        struct {
            const char* rgen;                       // Ray Generation Shader source code.
            const char* rgenName;                   // Ray Generation Shader entry point name.
            const char* rmiss;                      // Ray Miss Shader source code.
            const char* rmissName;                  // Ray Miss Shader entry point name.
            const char* rany;                       // Ray Anyhit shader source code.
            const char* ranyName;                   // Ray Anyhit shader entry point name.
            const char* rclosest;                   // Ray Closest hit shader source code.
            const char* rclosestName;               // Ray Closest hit shader entry point name.
            const char* rintersect;                 // Ray Intersect Shader source code.
            const char* rintersectName;             // Ray Intersect Shader entry point name.
        } raytrace;
    };
    // Each instance in this vector holds contents that define a permutation.
    std::vector<ShaderProgramPermutationDefinitionInstance> permutationDefinitions;
    // The high level language that is used by this program.
    ShaderLanguage language;
    // The pipeline type (graphics, compute, raytracing.)
    BindType pipelineType;

    ShaderProgramDescription() { memset(this, 0, sizeof(graphics)); }
    ShaderProgramDescription(const ShaderProgramDescription& description);
    ShaderProgramDescription(ShaderProgramDescription&& description);
    ~ShaderProgramDescription() { }
};

// Build the shader program definitions. When successful, will store the shader program into the given ShaderProgrmaDatabase. 
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