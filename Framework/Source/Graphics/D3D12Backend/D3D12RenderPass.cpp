//
#include "D3D12Resource.hpp"
#include "D3D12RenderPass.hpp"
#include "D3D12ResourceView.hpp"

#include "D3D12DescriptorTableManager.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Logger.hpp"

#include <unordered_map>

namespace Recluse {


std::unordered_map<Hash64, D3D12RenderPass> g_renderPassMap;
std::unordered_map<Hash64, CpuDescriptorTable> g_cachedRenderPassTable;


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


ResultCode D3D12RenderPass::update(D3D12Device* pDevice, U32 numRtvDescriptors, ResourceViewId* rtvDescriptors, ResourceViewId dsvDescriptor)
{
    R_ASSERT(pDevice != NULL);
    R_ASSERT_FORMAT(numRtvDescriptors <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, 
        "Number of render target binds can't be larger than the max! Max=%d, Requested=%d", D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, numRtvDescriptors);
    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();
    D3D12_CPU_DESCRIPTOR_HANDLE handles[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
    for (U32 i = 0; i < numRtvDescriptors; ++i)
    {
        if (rtvDescriptors[i] != 0)
        {
            D3D12GraphicsResourceView* pView = DescriptorViews::findResourceView(rtvDescriptors[i]);
            m_rtvFormats[i] = Dxgi::getNativeFormat(pView->getDesc().format);
            handles[i] = pView->getCpuDescriptor();
        }
        else
        {
            handles[i] = pDescriptorManager->nullRtvDescriptor();
            m_rtvFormats[i] = DXGI_FORMAT_UNKNOWN;
        }
    }
    
    Hash64 rtvHash = recluseHashFast(handles, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * numRtvDescriptors);
    auto iter = g_cachedRenderPassTable.find(rtvHash);
    if (iter == g_cachedRenderPassTable.end())
    {
        m_rtvDhAllocation = pDescriptorManager->copyDescriptorsToTable(CpuHeapType_Rtv, handles, numRtvDescriptors);
        g_cachedRenderPassTable.insert(std::make_pair(rtvHash, m_rtvDhAllocation.baseCpuDescriptorHandle));
    }
    else
    {
        m_rtvDhAllocation = iter->second;
    }
    if (dsvDescriptor != 0)
    {
        D3D12GraphicsResourceView* pDsv = DescriptorViews::findResourceView(dsvDescriptor);
        m_dsvFormat = Dxgi::getNativeFormat(pDsv->getDesc().format);
        m_dsvDhAllocation = pDsv->getCpuDescriptor();
    }
    else
    {
        m_dsvDhAllocation = pDescriptorManager->nullDsvDescriptor();
        m_dsvFormat = DXGI_FORMAT_UNKNOWN;
    }
    return RecluseResult_Ok;
}


ResultCode D3D12RenderPass::release(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);

    auto* pDescriptorManager = pDevice->getDescriptorHeapManager();
    R_ASSERT(pDescriptorManager != NULL);
    pDescriptorManager->freeDescriptorTable(CpuHeapType_Rtv, m_rtvDhAllocation);
    m_rtvDhAllocation.invalidate();
    return RecluseResult_NoImpl;
}


namespace RenderPasses {
D3D12RenderPass* makeRenderPass(D3D12Device* pDevice, U32 numRtvs, ResourceViewId* rtvs, ResourceViewId dsv)
{
    D3D12RenderPass* pass = nullptr;
    ResourceViewId targets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT + 1]; // For the dsv if we have one.
    
    U32 numTargets = numRtvs;
    memcpy(targets, rtvs, sizeof(ResourceViewId) * numRtvs);
    if (dsv != 0)
        targets[numTargets++] = dsv;

    Hash64 hash = recluseHashFast(targets, sizeof(ResourceViewId) * numTargets);
    auto iter = g_renderPassMap.find(hash);
    if (iter == g_renderPassMap.end())
    {
        D3D12RenderPass renderPass = { };
        renderPass.update(pDevice, numRtvs, rtvs, dsv);
        g_renderPassMap.insert(std::make_pair(hash, renderPass));
        pass = &g_renderPassMap[hash];
    }
    else
    {
        pass = &iter->second;
        pass->update(pDevice, numRtvs, rtvs, dsv);
    }
    return pass;
}


void clearAll(D3D12Device* pDevice)
{
    for (auto iter : g_renderPassMap)
    {
        iter.second.release(pDevice);
    }
    g_renderPassMap.clear();
}


void clearRenderPassCache()
{
    g_cachedRenderPassTable.clear();
}
} // RenderPasses
} // Recluse