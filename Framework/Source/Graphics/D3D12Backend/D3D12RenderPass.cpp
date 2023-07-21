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
    return m_rtvDhAllocation.getTotalDescriptors();
}


ResultCode D3D12RenderPass::update(D3D12Device* pDevice, U32 bufferIdx, U32 numRtvDescriptors, GraphicsResourceView* const* rtvDescriptors, const GraphicsResourceView* dsvDescriptor)
{
    R_ASSERT(pDevice != NULL);
    R_ASSERT(numRtvDescriptors <= 8);

    if (!shouldUpdate(bufferIdx))
    {
        return RecluseResult_Ok;
    }

    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();
    DescriptorHeapInstance* descriptorHeap = pDescriptorManager->getInstance(bufferIdx);

    R_ASSERT(pDescriptorManager != NULL);

    ID3D12Device* device = pDevice->get();
    D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor = { 0 };

    // allocate our rtv descriptors first.
    // TODO: We probably want to get rid of the const_cast parts, this is not very efficient.
    if (numRtvDescriptors)
    {
        DescriptorHeapAllocation dhAllocation = descriptorHeap->allocate(device, numRtvDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (U32 i = 0; i < numRtvDescriptors; ++i)
        {
            const D3D12GraphicsResourceView* pResourceView = rtvDescriptors[i]->castTo<D3D12GraphicsResourceView>();
            const D3D12_RENDER_TARGET_VIEW_DESC& rtvDescription = pResourceView->asRtv();
            ID3D12Resource* pResource = pResourceView->getResource()->castTo<D3D12Resource>()->get();
            device->CreateRenderTargetView(pResource, &rtvDescription, dhAllocation.getCpuDescriptor(i));
        }
        m_rtvDhAllocation = dhAllocation;
    }

    if (dsvDescriptor)
    {
        DescriptorHeapAllocation dsvAllocation = descriptorHeap->allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        const D3D12_DEPTH_STENCIL_VIEW_DESC& dsvDescription = dsvDescriptor->castTo<D3D12GraphicsResourceView>()->asDsv();
        ID3D12Resource* pResource = dsvDescriptor->getResource()->castTo<D3D12Resource>()->get();
        device->CreateDepthStencilView(pResource, &dsvDescription, dsvAllocation.getCpuDescriptor());
        m_dsvDhAllocation = dsvAllocation;
    }

    // Assign the updated buffer idx to let us know that we have updated this render pass.
    m_updateBufferIdx = bufferIdx;

    return RecluseResult_Ok;
}


ResultCode D3D12RenderPass::release(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);

    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();
    DescriptorHeapInstance* descriptorHeap = pDescriptorManager->getInstance(0);

    R_ASSERT(pDescriptorManager != NULL);

    if (m_rtvDhAllocation.isValid())
    {
        descriptorHeap->free(m_rtvDhAllocation);
    }

    if (m_dsvDhAllocation.isValid())
    {
        descriptorHeap->free(m_dsvDhAllocation);
    }

    return RecluseResult_NoImpl;
}


namespace RenderPasses {
D3D12RenderPass* makeRenderPass(U32 numRtvs, GraphicsResourceView* const* rtvs, const GraphicsResourceView* dsv)
{
    return nullptr;
}
} // RenderPasses
} // Recluse