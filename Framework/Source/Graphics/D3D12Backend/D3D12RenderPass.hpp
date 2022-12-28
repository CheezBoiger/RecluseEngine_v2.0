//
#pragma once

#include "D3D12Commons.hpp"
#include "D3D12Device.hpp"

#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "D3D12DescriptorTableManager.hpp"

namespace Recluse {


class D3D12RenderPass : public RenderPass
{
public:
    D3D12RenderPass()
    { }

    ErrType                         initialize(D3D12Device* pDevice);
    ErrType                         destroy(D3D12Device* pDevice);
    
    virtual U32                     getNumRenderTargets() const override;
    virtual GraphicsResourceView*   getRenderTarget(U32 idx) override;
    virtual GraphicsResourceView*   getDepthStencil() override;

    D3D12_CPU_DESCRIPTOR_HANDLE     getRtvHandle() const { return m_rtvDhAllocation.getCpuHandle(); }
    D3D12_CPU_DESCRIPTOR_HANDLE     getDsvHandle() const { return m_dsvDhAllocation.getCpuHandle(); }

private:
    DescriptorHeapAllocation    m_rtvDhAllocation;
    DescriptorHeapAllocation    m_dsvDhAllocation;
};
} // Recluse