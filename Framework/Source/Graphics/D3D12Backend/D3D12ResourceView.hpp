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
    GraphicsAPI getApi() const override { return GraphicsApi_Direct3D12; }

    D3D12GraphicsResourceView(const ResourceViewDescription& description)
        : GraphicsResourceView(description) { }
    ResultCode initialize(D3D12Device* pDevice, ID3D12Resource* pResource);
    ResultCode release(D3D12Device* pDevice);

    U64 getId() const { return m_id; }

    D3D12_CPU_DESCRIPTOR_HANDLE getCpuDescriptor() const { return m_handle; }

private:
    static U64 kViewCreationCounter;
    static MutexGuard kViewMutex;
    U64 m_id;
    D3D12_CPU_DESCRIPTOR_HANDLE m_handle;

    virtual void generateId() override
    {
        ScopedLock _(kViewMutex);
        m_id = (++kViewCreationCounter);
    }
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

ResourceViewId                  makeResourceView(D3D12Device* pDevice, ID3D12Resource* pResource, const ResourceViewDescription& description);
D3D12_CPU_DESCRIPTOR_HANDLE     makeCbv(D3D12Device* pDevice, D3D12_GPU_VIRTUAL_ADDRESS address, U32 sizeBytes);
D3D12GraphicsResourceView*      findResourceView(ResourceViewId id);
ResultCode                      destroyResourceView(D3D12Device* pDevice, ResourceViewId resourceId);
ResultCode                      destroyCbv(D3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE handle);
void                            clearAll(D3D12Device* pDevice);
} // DescriptorViews
} // Recluse