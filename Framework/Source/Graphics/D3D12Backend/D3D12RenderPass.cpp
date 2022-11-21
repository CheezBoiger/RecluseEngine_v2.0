//
#include "D3D12Resource.hpp"
#include "D3D12RenderPass.hpp"
#include "D3D12ResourceView.hpp"

#include "D3D12DescriptorTableManager.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Logger.hpp"

namespace Recluse {


GraphicsResourceView* D3D12RenderPass::getRenderTarget(U32 idx)
{
    return nullptr;
}


GraphicsResourceView* D3D12RenderPass::getDepthStencil()
{
    return nullptr;
}


U32 D3D12RenderPass::getNumRenderTargets() const
{
    return m_renderPassDesc.numRenderTargets;
}


ErrType D3D12RenderPass::initialize(D3D12Device* pDevice, const RenderPassDesc& desc)
{
    R_ASSERT(pDevice != NULL);

    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();

    R_ASSERT(pDescriptorManager != NULL);
    D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor = { 0 };

    // allocate our rtv descriptors first.
    DescriptorHeapAllocation dhAllocation = pDescriptorManager->allocate(desc.numRenderTargets, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (!dhAllocation.isValid())
    {
        return RecluseResult_Failed;
    }

    for (U32 i = 0; i < desc.numRenderTargets; ++i)
    {
        D3D12GraphicsResourceView* pResourceView        = static_cast<D3D12GraphicsResourceView*>(desc.ppRenderTargetViews[i]);
        D3D12_RENDER_TARGET_VIEW_DESC descRtv           = pResourceView->getRtvDesc();
        D3D12Resource* pResource                        = static_cast<D3D12Resource*>(pResourceView->getResource());

        R_ASSERT_MSG(pResource != NULL, "Resource for the given resource view is NULL!");
        D3D12_CPU_DESCRIPTOR_HANDLE destHandle = dhAllocation.getCpuHandle(i);

        pDevice->createRenderTargetView(pResource, descRtv, destHandle);
    }

    // If we have depth stencil, we will allocate to the dsv descriptor heap.
    if (desc.pDepthStencil)
    {
        if (desc.pDepthStencil->hasResource())
        {
            DescriptorHeapAllocation allocation         = pDescriptorManager->allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            D3D12GraphicsResourceView* pResourceView    = static_cast<D3D12GraphicsResourceView*>(desc.pDepthStencil);
            D3D12_DEPTH_STENCIL_VIEW_DESC descDsv       = pResourceView->getDsvDesc();
            D3D12Resource* pResource                    = static_cast<D3D12Resource*>(pResourceView->getResource());

            pDevice->createDepthStencilView(pResource, descDsv, allocation.getCpuHandle());
        }
    }

    return RecluseResult_Ok;
}


ErrType D3D12RenderPass::destroy(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);

    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();

    R_ASSERT(pDescriptorManager != NULL);

    if (m_rtvDhAllocation.isValid())
    {
        pDescriptorManager->free(m_rtvDhAllocation);
    }

    if (m_dsvDhAllocation.isValid())
    {
        pDescriptorManager->free(m_dsvDhAllocation);
    }

    return RecluseResult_NoImpl;
}
} // Recluse