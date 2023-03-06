//
#pragma once

#include "D3D12Commons.hpp"
#include "D3D12Device.hpp"

#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "D3D12DescriptorTableManager.hpp"

namespace Recluse {

class D3D12GraphicsResourceView;

class D3D12RenderPass
{
public:
    D3D12RenderPass()
    { }

    ErrType initialize
        (
            D3D12Device* pDevice, 
            U32 numRtvDescriptors, 
            const D3D12GraphicsResourceView** rtvDescriptors, 
            const D3D12GraphicsResourceView* dsvDescriptor = nullptr
        );

    ErrType release(D3D12Device* pDevice);
    
    U32                     getNumRenderTargets() const;
    GraphicsResourceView*   getRenderTarget(U32 idx);
    GraphicsResourceView*   getDepthStencil();

    D3D12_CPU_DESCRIPTOR_HANDLE     getRtvDescriptor(U32 idx = 0u) const { return m_rtvDhAllocation.getCpuDescriptor(idx); }
    D3D12_CPU_DESCRIPTOR_HANDLE     getDsvDescriptor() const { return m_dsvDhAllocation.getCpuDescriptor(); }

private:
    DescriptorHeapAllocation    m_rtvDhAllocation;
    DescriptorHeapAllocation    m_dsvDhAllocation;
};
} // Recluse