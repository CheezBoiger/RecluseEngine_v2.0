//
#include "D3D12RenderPass.hpp"

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