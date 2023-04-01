//
#pragma once

#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Types.hpp"
#include "D3D12Commons.hpp"
#include "Recluse/Graphics/ShaderProgramBuilder.hpp"

namespace Recluse {

class D3D12Context;


extern const char* kHlslSemanticPosition;
extern const char* kHlslSemanticNormal;
extern const char* kHlslSemanticTexcoord;
extern const char* kHlslSemanticTangent;

namespace Pipelines {


struct PipelineStateObject
{
    BindType pipelineType;
    union 
    {
        struct 
        {
            U32                 numRenderTargets;
            RasterState         raster;
            BlendState          blendState;
            DepthStencil        depthStencil;
            VertexInputLayoutId ia;
            TessellationState   tess;
            PrimitiveTopology   primitiveTopology;
        } graphics;
        struct
        {
            ID3D12RootSignature*    localRootSignature;
            Bool                    usingLocalRootSignature;               //< If using local root signature, otherwise use global.
        } raytrace;
    };
    ShaderProgramId             shaderProgramId;
    ShaderProgramPermutation    permutation;
    ID3D12RootSignature*        rootSignature; 
};

ID3D12PipelineState* makePipelineState(D3D12Context* pContext, const PipelineStateObject& pipelineState);

} // Pipelines
} // Recluse