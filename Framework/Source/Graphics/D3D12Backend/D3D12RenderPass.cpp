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
    return 0u;
}


ResultCode D3D12RenderPass::update(D3D12Device* pDevice, U32 bufferIdx, U32 numRtvDescriptors, ResourceViewId* rtvDescriptors, ResourceViewId dsvDescriptor)
{
    R_ASSERT(pDevice != NULL);
    R_ASSERT(numRtvDescriptors <= 8);

    if (!shouldUpdate(bufferIdx))
    {
        return RecluseResult_Ok;
    }

    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();

    // Assign the updated buffer idx to let us know that we have updated this render pass.
    m_updateBufferIdx = bufferIdx;

    return RecluseResult_Ok;
}


ResultCode D3D12RenderPass::release(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);

    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();

    R_ASSERT(pDescriptorManager != NULL);    

    return RecluseResult_NoImpl;
}


namespace RenderPasses {
D3D12RenderPass* makeRenderPass(U32 numRtvs, ResourceViewId* rtvs, ResourceViewId dsv)
{
    return nullptr;
}
} // RenderPasses
} // Recluse