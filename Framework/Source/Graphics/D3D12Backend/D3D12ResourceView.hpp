//
#pragma once

#include "Recluse/Graphics/ResourceView.hpp"
#include "D3D12Commons.hpp"
#include "D3D12DescriptorTableManager.hpp"

namespace Recluse {

class D3D12Device;

class D3D12GraphicsResourceView : public GraphicsResourceView
{
public:

    ResultCode initialize(D3D12Device* pDevice);
    ResultCode cleanUp(D3D12Device* pDevice);

    U64 getDescId() const { return m_descId; }

    const D3D12_RENDER_TARGET_VIEW_DESC& asRtv() const { return rtvDescription; }
    const D3D12_DEPTH_STENCIL_VIEW_DESC& asDsv() const { return dsvDescription; }
    const D3D12_UNORDERED_ACCESS_VIEW_DESC& asUav() const { return uavDescription; }
    const D3D12_SHADER_RESOURCE_VIEW_DESC& asSrv() const { return srvDescription; }

private:
    static U64 kViewCreationCounter;
    static MutexGuard kViewMutex;
    U64 m_descId;

    virtual void generateId() override
    {
        ScopedLock _(kViewMutex);
        m_descId = kViewCreationCounter++;
    }

    union
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDescription;
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescription;
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescription;
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription;
    };
};


class D3D12Sampler : public GraphicsSampler 
{
public:
    D3D12Sampler() { }
    ~D3D12Sampler() { }
    
    ResultCode initialize(D3D12Device* pDevice, const SamplerDescription& desc);
    ResultCode destroy(D3D12Device* pDevice);

    D3D12_SAMPLER_DESC get() { return m_samplerDesc; }

private:
    D3D12_SAMPLER_DESC m_samplerDesc;
};


namespace DescriptorViews {

D3D12GraphicsResourceView*  makeResourceView();
ResultCode                     destroyResourceView();
} // DescriptorViews
} // Recluse