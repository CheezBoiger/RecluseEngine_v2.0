//
#pragma once

#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Types.hpp"
#include "D3D12Commons.hpp"
#include "Recluse/Graphics/ShaderProgramBuilder.hpp"

namespace Recluse {

class D3D12Context;
class D3D12Device;

extern const char* kHlslSemanticPosition;
extern const char* kHlslSemanticNormal;
extern const char* kHlslSemanticTexcoord;
extern const char* kHlslSemanticTangent;

typedef Hash64 PipelineStateId;

namespace Pipelines {


struct PipelineStateObject
{
    BindType                        pipelineType;
    union 
    {
        struct 
        {
            U32                     numRenderTargets;
            RasterState             raster;
            BlendState              blendState;
            DepthStencil            depthStencil;
            VertexInputLayoutId     ia;
            TessellationState       tess;
            PrimitiveTopology       primitiveTopology;
        } graphics;
        struct
        {
            ID3D12RootSignature*    localRootSignature;
            Bool                    usingLocalRootSignature;               //< If using local root signature, otherwise use global.
        } raytrace;
    };
    ShaderProgramId                 shaderProgramId;
    ShaderProgramPermutation        permutation;
    ID3D12RootSignature*            rootSignature; 
};


struct ConstantBufferView
{
    D3D12_GPU_VIRTUAL_ADDRESS address;
    U32                       offsetBytes;
    U32                       sizeBytes;
};

struct RootSigLayout
{
    U16                     cbvCount;
    U16                     srvCount;
    U16                     uavCount;
    U16                     samplerCount;
    ShaderStageFlags        shaderVisibility;
};


struct RootSigResourceTable
{   
    D3D12_CPU_DESCRIPTOR_HANDLE*    cbvs;
    D3D12_CPU_DESCRIPTOR_HANDLE*    srvs;
    D3D12_CPU_DESCRIPTOR_HANDLE*    uavs;
    D3D12_CPU_DESCRIPTOR_HANDLE*    samplers;   
};

ID3D12PipelineState*            makePipelineState(D3D12Context* pContext, const PipelineStateObject& pipelineState);
ID3D12RootSignature*            makeRootSignature(D3D12Device* pDevice, const RootSigLayout& layout);
CpuDescriptorTable              makeDescriptorSrvCbvUavTable(D3D12Device* pDevice, const RootSigLayout& layout, const RootSigResourceTable& resourceTable);
CpuDescriptorTable              makeDescriptorSamplertable(D3D12Device* pDevice, const RootSigLayout& layout, const RootSigResourceTable& resourceTable);
void                            cleanUpRootSigs();
} // Pipelines
} // Recluse