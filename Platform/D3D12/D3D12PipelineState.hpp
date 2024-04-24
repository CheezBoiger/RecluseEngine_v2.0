//
#pragma once

#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Types.hpp"
#include "D3D12Commons.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"
#include "D3D12ShaderCache.hpp"

#include <vector>

namespace Recluse {
namespace D3D12 {
class D3D12Context;
class D3D12Device;
class D3D12RenderPass;

typedef Hash64 PipelineStateId;

namespace Pipelines {
namespace VertexInputs {


struct D3DVertexInput
{
    std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
    std::vector<U32> vertexByteStrides;
};


ResultCode make(DeviceId deviceId, VertexInputLayoutId id, const VertexInputLayout& layout);
ResultCode unload(DeviceId deviceId, VertexInputLayoutId id);
D3DVertexInput* obtain(DeviceId deviceId, VertexInputLayoutId id);
Bool unloadAll(DeviceId deviceId);
} // VertexInputs


struct PipelineStateObject
{
    BindType                        pipelineType;
    union 
    {
        struct 
        {
            U32                                 numRenderTargets;
            RasterState                         raster;
            BlendState                          blendState;
            DepthStencil                        depthStencil;
            VertexInputLayoutId                 inputLayoutId;
            TessellationState                   tess;
            D3D12_PRIMITIVE_TOPOLOGY_TYPE       topologyType;
            PolygonMode                         polygonMode;
            CullMode                            cullMode;
            FrontFace                           frontFace;
            Bool                                antiAliasedLineEnable;
            Bool                                depthBiasEnable;
            Bool                                depthClampEnable;
            DXGI_FORMAT                         rtvFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
            DXGI_FORMAT                         dsvFormat;
            D3D12_INDEX_BUFFER_STRIP_CUT_VALUE  indexStripCut;
        } graphics;
        struct
        {
            ID3D12RootSignature*    localRootSignature;
            Bool                    usingLocalRootSignature;               //< If using local root signature, otherwise use global.
            U32                     rayRecursionDepth;
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
    U16                         cbvCount;
    U16                         srvCount;
    U16                         uavCount;
    U16                         samplerCount;
    ShaderStageFlags            shaderVisibility;
    D3D12_ROOT_SIGNATURE_FLAGS  flags;
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
void                            cleanUpRootSigs(DeviceId deviceId);
void                            cleanUpPipelines(DeviceId deviceId);
void                            resetTableHeaps(D3D12Device* pDevice);
void                            updateT(D3D12Device* pDevice);
void                            checkPipelines(D3D12Device* pDevice);
} // Pipelines
} // D3D12
} // Recluse