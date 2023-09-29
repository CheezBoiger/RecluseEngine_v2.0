//
#include "D3D12Resource.hpp"
#include "D3D12RenderPass.hpp"
#include "D3D12ResourceView.hpp"

#include "D3D12DescriptorTableManager.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Logger.hpp"
#include "Graphics/LifetimeCache.hpp"

#include <unordered_map>

namespace Recluse {
namespace D3D12 {

LifetimeCache<Hash64, D3D12RenderPass> g_renderPassMap;
std::unordered_map<Hash64, CpuDescriptorTable> g_cachedRenderPassTable;


R_DECLARE_GLOBAL_U32(g_d3d12RenderPassMaxAge, 12u, "D3D12.RenderPassMaxAge");


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
    if (!g_renderPassMap.inCache(hash))
    {
        D3D12RenderPass renderPass = { };
        renderPass.update(pDevice, numRtvs, rtvs, dsv);
        pass = g_renderPassMap.insert(hash, std::move(renderPass));
    }
    else
    {
        pass = g_renderPassMap.refer(hash);
        pass->update(pDevice, numRtvs, rtvs, dsv);
    }
    return pass;
}


void clearAll(D3D12Device* pDevice)
{
    g_renderPassMap.forEach([pDevice] (D3D12RenderPass& renderPass) -> void
        {
            R_DEBUG(R_CHANNEL_D3D12, "Cleaning up renderpass.");
            renderPass.release(pDevice);
        });
    g_renderPassMap.clear();
    g_cachedRenderPassTable.clear();
}


void clearRenderPassCache()
{
    g_cachedRenderPassTable.clear();
}


void sweep(D3D12Device* pDevice)
{
    // Perform a check to sweep up any old render pass.
    g_renderPassMap.check(g_d3d12RenderPassMaxAge, [pDevice] (D3D12RenderPass& renderPass) -> void 
        {
            R_DEBUG(R_CHANNEL_D3D12, "Cleaning up renderpass.");
            renderPass.release(pDevice);
        });
}


void update()
{
    g_renderPassMap.updateTick();
}
} // RenderPasses
} // D3D12
} // Recluse