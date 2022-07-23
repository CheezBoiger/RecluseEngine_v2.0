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

    for (U32 i = 0; i < desc.numRenderTargets; ++i)
    {
        D3D12_RENDER_TARGET_VIEW_DESC descRtv           = {};
        D3D12GraphicsResourceView* pResourceView        = static_cast<D3D12GraphicsResourceView*>(desc.ppRenderTargetViews[i]);
        D3D12Resource* pResource                        = static_cast<D3D12Resource*>(pResourceView->getResource());

        R_ASSERT_MSG(pResource != NULL, "Resource for the given resource view is NULL!");

        baseDescriptor = pDescriptorManager->createRtv(descRtv, pResource->get());
    }
    pDescriptorManager;

    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType D3D12RenderPass::destroy(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);

    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();

    R_ASSERT(pDescriptorManager != NULL);

    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse