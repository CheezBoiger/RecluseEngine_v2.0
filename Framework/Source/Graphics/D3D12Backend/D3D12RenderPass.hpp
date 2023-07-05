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

    // Update the render pass with latest descriptors for use. Be sure to call this every time we use the next buffer.
    // This is required to prevent stale descriptors.
    // This function will also check if it needs to update, should it have already been updated.
    ResultCode                      update
        (
            D3D12Device* pDevice,
            U32 bufferIdx,
            U32 numRtvDescriptors, 
            const D3D12GraphicsResourceView** rtvDescriptors, 
            const D3D12GraphicsResourceView* dsvDescriptor = nullptr
        );

    ResultCode                      release(D3D12Device* pDevice);
    U32                             getNumRenderTargets() const;
    GraphicsResourceView*           getRenderTarget(U32 idx);
    GraphicsResourceView*           getDepthStencil();

    D3D12_CPU_DESCRIPTOR_HANDLE     getRtvDescriptor(U32 idx = 0u) const { return m_rtvDhAllocation.getCpuDescriptor(idx); }
    D3D12_CPU_DESCRIPTOR_HANDLE     getDsvDescriptor() const { return m_dsvDhAllocation.getCpuDescriptor(); }

    // 
    B32                             shouldUpdate(U32 bufferIdx) const { return (bufferIdx != m_updateBufferIdx); }

private:
    DescriptorHeapAllocation    m_rtvDhAllocation;
    DescriptorHeapAllocation    m_dsvDhAllocation;
    U32                         m_updateBufferIdx;
};

namespace RenderPasses {


D3D12RenderPass* makeRenderPass(U32 numRtvs, const GraphicsResourceView** rtvs, const GraphicsResourceView* dsv = nullptr);
} // Renderpasses
} // Recluse