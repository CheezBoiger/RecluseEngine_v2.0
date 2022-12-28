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

    U64 getDescId() const { return m_descId; }
private:
    U64 m_descId;
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


namespace DescriptorViews {

D3D12GraphicsResourceView*  makeResourceView();
ErrType                     destroyResourceView();
} // DescriptorViews
} // Recluse