//
#pragma once

#include "Win32/Win32Common.hpp"

#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/Shader.hpp"

// D3D12 headers.
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>

#define R_CHANNEL_D3D12 "D3D12"

// Useful information about differences between D3D12 and Vulkan:
// https://asawicki.info/articles/memory_management_vulkan_direct3d_12.php5
// Proc access to these functions when we query for instance.


namespace Recluse {


class D3D12GraphicsObject : public IGraphicsObject
{
public:
	virtual ~D3D12GraphicsObject() { }

	GraphicsAPI getApi() const override { return GraphicsApi_Direct3D12; }
};


struct D3D12MemoryPool 
{
    ID3D12Heap* pHeap;
    U64             sizeInBytes;
};


struct D3D12MemoryObject 
{
    ID3D12Resource*         pResource;
    U64                     sizeInBytes;
    UPtr                    basePtr;
    U32                     allocatorIndex;
    ResourceMemoryUsage     usage;
};


// Get the native resource state.
extern D3D12_RESOURCE_STATES                getNativeResourceState(Recluse::ResourceState state);
extern D3D12_RTV_DIMENSION                  getRtvDimension(Recluse::ResourceViewDimension dimension);
extern D3D12_DSV_DIMENSION                  getDsvDimension(Recluse::ResourceViewDimension dimension);
extern D3D12_UAV_DIMENSION                  getUavDimension(Recluse::ResourceViewDimension dimension);
extern D3D12_SRV_DIMENSION                  getSrvDimension(Recluse::ResourceViewDimension dimension);
extern D3D12_COMPARISON_FUNC                getNativeComparisonFunction(Recluse::CompareOp compareOp);
extern D3D12_BLEND_OP                       getBlendOp(Recluse::BlendOp blendOp);
extern D3D12_PRIMITIVE_TOPOLOGY             getPrimitiveTopology(Recluse::PrimitiveTopology topology);
extern D3D12_PRIMITIVE_TOPOLOGY_TYPE        getPrimitiveTopologyType(PrimitiveTopology topology);
extern D3D12_BLEND                          getBlendFactor(Recluse::BlendFactor blendFactor);
extern D3D12_LOGIC_OP                       getLogicOp(Recluse::LogicOp logicOp);
extern D3D12_STENCIL_OP                     getStencilOp(Recluse::StencilOp stencilOp);
extern D3D12_SHADER_VISIBILITY              getShaderVisibilityFlags(Recluse::ShaderStageFlags shaderStageFlags);
extern D3D12_INDEX_BUFFER_STRIP_CUT_VALUE   getNativeStripCutValue(Recluse::IndexType indexType);
extern UINT                                 calculateSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize);

} // Recluse


namespace Dxgi {

// Check format size, this uses DXGI format.
extern SIZE_T getNativeFormatSize(DXGI_FORMAT format);

// Get the native format.
extern DXGI_FORMAT getNativeFormat(Recluse::ResourceFormat format);
} // Dxgi


// Coded Recluse mesh shader pipeline description.
// We are following https://github.com/microsoft/DirectX-Headers/blob/main/include/directx/d3dx12_pipeline_state_stream.h
// and not including the d3dx12.h headers from this repo, because we don't want to rely on
// helpers for d3d12.
struct R_D3D12_MESH_SHADER_PIPELINE_STATE_DESC
{
    ID3D12RootSignature*          pRootSignature;
    D3D12_SHADER_BYTECODE         AS;
    D3D12_SHADER_BYTECODE         MS;
    D3D12_SHADER_BYTECODE         PS;
    D3D12_BLEND_DESC              BlendState;
    UINT                          SampleMask;
    D3D12_RASTERIZER_DESC         RasterizerState;
    D3D12_DEPTH_STENCIL_DESC      DepthStencilState;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
    UINT                          NumRenderTargets;
    DXGI_FORMAT                   RTVFormats[ D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT ];
    DXGI_FORMAT                   DSVFormat;
    DXGI_SAMPLE_DESC              SampleDesc;
    UINT                          NodeMask;
    D3D12_CACHED_PIPELINE_STATE   CachedPSO;
    D3D12_PIPELINE_STATE_FLAGS    Flags;
};


class D3D12DepthStencilState1
{
public:
    static D3D12_DEPTH_STENCIL_DESC1 makeDepthStencil1(const D3D12_DEPTH_STENCIL_DESC& desc)
    {
        D3D12_DEPTH_STENCIL_DESC1 desc1;
        desc1.BackFace = desc.BackFace;
        desc1.DepthEnable = desc.DepthEnable;
        desc1.DepthFunc = desc.DepthFunc;
        desc1.DepthWriteMask = desc.DepthWriteMask;
        desc1.FrontFace = desc.FrontFace;
        desc1.StencilEnable = desc.StencilEnable;
        desc1.StencilReadMask = desc.StencilReadMask;
        desc1.StencilWriteMask = desc.StencilWriteMask;
        desc1.DepthBoundsTestEnable = false;
        return desc1;
    }
};


template<typename DescInfo, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE subobjectType, typename DefaultParam = DescInfo>
struct alignas(void*) StreamDescription
{
private:
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type;
public:
    DescInfo info;

    StreamDescription() noexcept : type(subobjectType), info(DefaultParam()) { }
    StreamDescription(const DescInfo& value) noexcept : type(subobjectType), info(value) { }
    StreamDescription& operator=(const DescInfo& value) noexcept { type = subobjectType; info = value; return (*this); }
};


struct RD3D12MeshShaderStreamDescription
{
    RD3D12MeshShaderStreamDescription(const R_D3D12_MESH_SHADER_PIPELINE_STATE_DESC& desc)
        : flags(desc.Flags)
        , nodeMask(desc.NodeMask)
        , rootSig(desc.pRootSignature)
        , PS(desc.PS)
        , AS(desc.AS)
        , MS(desc.MS)
        , blendState(desc.BlendState)
        , depthStencilState(D3D12DepthStencilState1::makeDepthStencil1(desc.DepthStencilState))
        , dsvFormat(desc.DSVFormat)
        , rasterizer(desc.RasterizerState)
        , sampleDesc(desc.SampleDesc)
        , sampleMask(desc.SampleMask)
        , cachedPSO(desc.CachedPSO)
    {
        rtvFormats.info.NumRenderTargets = desc.NumRenderTargets;
        for (UINT i = 0; i < desc.NumRenderTargets; ++i)
            rtvFormats.info.RTFormats[i] = desc.RTVFormats[i];
    }
    StreamDescription<D3D12_PIPELINE_STATE_FLAGS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS> flags;
    StreamDescription<UINT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK> nodeMask;
    StreamDescription<ID3D12RootSignature*, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE> rootSig;
    StreamDescription<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS> PS;
    StreamDescription<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS> AS;
    StreamDescription<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS> MS;
    StreamDescription<D3D12_BLEND_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND> blendState;
    StreamDescription<D3D12_DEPTH_STENCIL_DESC1, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1> depthStencilState;
    StreamDescription<DXGI_FORMAT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT> dsvFormat;
    StreamDescription<D3D12_RASTERIZER_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER> rasterizer;
    StreamDescription<D3D12_RT_FORMAT_ARRAY, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS> rtvFormats;
    StreamDescription<DXGI_SAMPLE_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC> sampleDesc;
    StreamDescription<UINT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK> sampleMask;
    StreamDescription<D3D12_CACHED_PIPELINE_STATE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO> cachedPSO;
};