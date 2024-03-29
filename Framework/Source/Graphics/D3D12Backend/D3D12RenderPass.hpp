//
#pragma once

#include "D3D12Commons.hpp"
#include "D3D12Device.hpp"

#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "D3D12DescriptorTableManager.hpp"

namespace Recluse {
namespace D3D12 {

class D3D12GraphicsResourceView;

class D3D12RenderPass
{
public:
    D3D12RenderPass()
        : m_rtvDhAllocation(DescriptorTable::invalidCpuAddress)
        , m_dsvDhAllocation(DescriptorTable::invalidCpuAddress)
        , m_dsvFormat(DXGI_FORMAT_UNKNOWN)
    { }

    ResultCode                      update
        (
            D3D12Device* pDevice,
            U32 numRtvDescriptors, 
            ResourceViewId* rtvDescriptors, 
            ResourceViewId dsvDescriptor = -1
        );

    ResultCode                      release(D3D12Device* pDevice);
    U32                             getNumRenderTargets() const;
    GraphicsResourceView*           getRenderTarget(U32 idx);
    GraphicsResourceView*           getDepthStencil();

    D3D12_CPU_DESCRIPTOR_HANDLE     getRtvDescriptor(U32 idx = 0u) const { return m_rtvDhAllocation.getAddress(idx); }
    D3D12_CPU_DESCRIPTOR_HANDLE     getDsvDescriptor() const { return m_dsvDhAllocation.baseCpuDescriptorHandle; }

    DXGI_FORMAT                     getRtvFormat(U32 index) const { return m_rtvFormats[index]; }
    DXGI_FORMAT                     getDsvFormat() const { return m_dsvFormat; }

private:
    CpuDescriptorTable          m_rtvDhAllocation;
    CpuDescriptorTable          m_dsvDhAllocation;
    DXGI_FORMAT                 m_rtvFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
    DXGI_FORMAT                 m_dsvFormat;
    
};

namespace RenderPasses {

void                clearAll(D3D12Device* pDevice);
D3D12RenderPass*    makeRenderPass(D3D12Device* pDevice, U32 numRtvs, ResourceViewId* rtvs, ResourceViewId dsv = 0);
void                clearRenderPassCache();
void                update();
void                sweep(D3D12Device* pDevice);
} // Renderpasses
} // D3D12
} // Recluse