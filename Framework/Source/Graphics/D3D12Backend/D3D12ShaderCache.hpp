//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"
#include "Recluse/Utility.hpp"

namespace Recluse {

class D3D12Device;

namespace D3D {

namespace Cache {


struct D3DShaderBytecode
{
    void*                       ptr;
    U64                         sizeBytes;
    Hash64                      shaderId;
    ShaderProgramPermutation    permutation;
};

struct D3DShaderProgram
{
    BindType bindType;

    union 
    {    
        struct
        {
            D3DShaderBytecode* psBytecode;
            const char* psMainEntry;
            union
            {
                struct
                {
                    D3DShaderBytecode* asBytecode;
                    const char* asMainEntry;
                    D3DShaderBytecode* msBytecode;
                    const char* msMainEntry;
                };
                struct
                {
                    D3DShaderBytecode* vsBytecode;
                    const char* vsMainEntry;
                    D3DShaderBytecode* hsBytecode;
                    const char* hsMainEntry;
                    D3DShaderBytecode* dsBytecode;
                    const char* dsMainEntry;
                    D3DShaderBytecode* gsBytecode;
                    const char* gsMainEntry;
                };
            };
            Bool usesMeshShaders;
        } graphics;
        struct
        {
            D3DShaderBytecode* csBytecode;
            const char* csMainEntry;
        } compute;
        struct
        {
            D3DShaderBytecode*   rayIntersect;
            const char* rayIntersectMainEntry;
            D3DShaderBytecode*   rayAny;
            const char* rayAnyMainEntry;
            D3DShaderBytecode*   rayGen;
            const char* rayGenEntryMainEntry;
            D3DShaderBytecode*   rayMiss;
            const char* rayMissMainEntry;
            D3DShaderBytecode*   rayClosest;
            const char* rayClosetEntryMainEntry;
        } raytrace;
    };
};


Bool                        isProgramCached(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);
Bool                        isProgramCached(ShaderProgramId shaderProgram);

// Returns a native shader program to the caller. If no native shader program is found, will return a nullptr.
D3DShaderProgram*           obtainShaderProgram(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);
ShaderProgramReflection*    obtainShaderProgramReflection(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);

ResultCode                  loadNativeShaderProgramPermutation(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition);
ResultCode                  unloadAll();
ResultCode                  unloadPrograms(ShaderProgramId shaderProgram);
} // Cache
} // D3D12 
} // Recluse