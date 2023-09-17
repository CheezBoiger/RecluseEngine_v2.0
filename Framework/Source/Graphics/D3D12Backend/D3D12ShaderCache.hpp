//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"

namespace Recluse {

class D3D12Device;

namespace D3D {

namespace Cache {

struct D3DShaderProgram
{
    BindType bindType;

    union 
    {    
        struct
        {
            ID3DBlob* psBytecode;
            const char* psMainEntry;
            union
            {
                struct
                {
                    ID3DBlob* asBytecode;
                    const char* asMainEntry;
                    ID3DBlob* msBytecode;
                    const char* msMainEntry;
                };
                struct
                {
                    ID3DBlob* vsBytecode;
                    const char* vsMainEntry;
                    ID3DBlob* hsBytecode;
                    const char* hsMainEntry;
                    ID3DBlob* dsBytecode;
                    const char* dsMainEntry;
                    ID3DBlob* gsBytecode;
                    const char* gsMainEntry;
                };
            };
            Bool usesMeshShaders;
        } graphics;
        struct
        {
            ID3DBlob* csBytecode;
            const char* csMainEntry;
        } compute;
        struct
        {
            ID3DBlob*   rayIntersect;
            const char* rayIntersectMainEntry;
            ID3DBlob*   rayAny;
            const char* rayAnyMainEntry;
            ID3DBlob*   rayGen;
            const char* rayGenEntryMainEntry;
            ID3DBlob*   rayMiss;
            const char* rayMissMainEntry;
            ID3DBlob*   rayClosest;
            const char* rayClosetEntryMainEntry;
        } raytrace;
    };
};


Bool                isProgramCached(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);
Bool                isProgramCached(ShaderProgramId shaderProgram);

// Returns a native shader program to the caller. If no native shader program is found, will return a nullptr.
D3DShaderProgram*   obtainShaderProgram(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation);

ResultCode          loadNativeShaderProgramPermutation(ShaderProgramId shaderProgram, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition);
ResultCode          unloadAll();
ResultCode          unloadPrograms(ShaderProgramId shaderProgram);
} // Cache
} // D3D12 
} // Recluse