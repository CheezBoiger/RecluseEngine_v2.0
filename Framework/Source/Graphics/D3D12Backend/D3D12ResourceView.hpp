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

    D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle() { return m_viewHandle; } 
private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_viewHandle;
};


class D3D12Sampler : public GraphicsSampler 
{
public:
    D3D12Sampler()
        : m_samplerCPUAddr({ 0 }) { }
    ~D3D12Sampler() { }
    
    ErrType initialize(D3D12Device* pDevice, const SamplerCreateDesc& desc);
    ErrType destroy(D3D12Device* pDevice);

    D3D12_CPU_DESCRIPTOR_HANDLE get() { return m_samplerCPUAddr; }

private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_samplerCPUAddr;
};
} // Recluse