//
#pragma once

#include "Recluse/Graphics/ResourceView.hpp"
#include "D3D12Commons.hpp"
#include "D3D12DescriptorTableManager.hpp"

namespace Recluse {


class D3D12GraphicsResourceView : public GraphicsResourceView
{
public:

    ErrType initialize(D3D12Device* pDevice);
    ErrType cleanUp(D3D12Device* pDevice);

    D3D12_RENDER_TARGET_VIEW_DESC getRtvDesc() const { return m_rtvDesc; }
    D3D12_DEPTH_STENCIL_VIEW_DESC getDsvDesc() const { return m_dsvDesc; }

private:
    union {
        D3D12_RENDER_TARGET_VIEW_DESC m_rtvDesc;
        D3D12_DEPTH_STENCIL_VIEW_DESC m_dsvDesc;
        D3D12_UNORDERED_ACCESS_VIEW_DESC m_uavDesc;
        D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
        D3D12_CONSTANT_BUFFER_VIEW_DESC m_cbvDesc;
    };
};


class D3D12Sampler : public GraphicsSampler 
{
public:
    D3D12Sampler() { }
    ~D3D12Sampler() { }
    
    ErrType initialize(D3D12Device* pDevice, const SamplerCreateDesc& desc);
    ErrType destroy(D3D12Device* pDevice);

    D3D12_SAMPLER_DESC get() { return m_samplerDesc; }

private:
    D3D12_SAMPLER_DESC m_samplerDesc;
};
} // Recluse