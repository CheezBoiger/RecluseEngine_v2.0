//
#pragma once

#include "D3D12Commons.hpp"
#include "D3D12Device.hpp"

#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

namespace Recluse {


class D3D12RenderPass : public RenderPass
{
public:
    D3D12RenderPass()
        : m_rtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE{0ull})
        , m_dsvHandle(D3D12_CPU_DESCRIPTOR_HANDLE{0ull})
    { }

    ErrType initialize(D3D12Device* pDevice, const RenderPassDesc& desc);
    ErrType destroy(D3D12Device* pDevice);
    
    virtual U32 getNumRenderTargets() const override;
    virtual GraphicsResourceView* getRenderTarget(U32 idx) override;
    virtual GraphicsResourceView* getDepthStencil() override;

    D3D12_CPU_DESCRIPTOR_HANDLE getRtvHandle() const { return m_rtvHandle; }
    D3D12_CPU_DESCRIPTOR_HANDLE getDsvHandle() const { return m_dsvHandle; }

private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle;
    RenderPassDesc              m_renderPassDesc;
};
} // Recluse