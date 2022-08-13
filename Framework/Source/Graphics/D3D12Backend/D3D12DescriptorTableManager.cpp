//
#include "D3D12DescriptorTableManager.hpp"
#include "D3D12Device.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12Device* pDevice)
    : m_pDevice(pDevice)
    , m_pCPUDescriptorHeap(nullptr)
    , m_pGPUDescriptorHeap(nullptr)
    , m_descHeapType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
    , m_descIncSz(0)
{
}


ErrType D3D12DescriptorHeap::initialize(D3D12_DESCRIPTOR_HEAP_TYPE type, U32 entries)
{
    R_ASSERT(m_pDevice != NULL);

    HRESULT result                      = S_OK;
    ID3D12Device* pDevice               = m_pDevice->get();
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = { };
    heapDesc.NodeMask                   = 0;
    heapDesc.NumDescriptors             = entries;
    heapDesc.Type                       = type;

    heapDesc.Flags  = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    result          = pDevice->CreateDescriptorHeap(&heapDesc, __uuidof(ID3D12DescriptorHeap),
                                            (void**)&m_pCPUDescriptorHeap);
    if (FAILED(result)) {

        R_ERR(R_CHANNEL_D3D12, "Failed to create descriptor heap.");

        return R_RESULT_FAILED;
    }

    // Create GPU descriptor heap ONLY if the descriptor heap is not rtv or dsv types.
    if (type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV && type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
        heapDesc.Flags  = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;    
        result          = pDevice->CreateDescriptorHeap
                                        (
                                            &heapDesc, 
                                            __uuidof(ID3D12DescriptorHeap),
                                            (void**)&m_pGPUDescriptorHeap
                                        );

        if (FAILED(result)) {

            R_ERR(R_CHANNEL_D3D12, "Failed to create shader visible descriptor heap.");

            return R_RESULT_FAILED;
        }
    }

    m_descHeapType  = type;
    m_descIncSz     = pDevice->GetDescriptorHandleIncrementSize(type);
    m_maxEntries    = entries;

    return R_RESULT_OK;
}


ErrType D3D12DescriptorHeap::destroy()
{
    R_ASSERT(m_pDevice != NULL);

    if (m_pCPUDescriptorHeap) {
        m_pCPUDescriptorHeap->Release();
        m_pCPUDescriptorHeap = nullptr;
    }

    if (m_pGPUDescriptorHeap) {
        m_pGPUDescriptorHeap->Release();
        m_pGPUDescriptorHeap = nullptr;
    }

    return R_RESULT_OK;
}


ErrType D3D12DescriptorHeap::update(U32 startEntry, U32 endEntry)
{
    R_ASSERT(m_pDevice != NULL);

    ID3D12Device* pDevice                       = m_pDevice->get();
    U32 totalEntries                            = endEntry - startEntry;
    U64 endEntry64                              = static_cast<U64>(endEntry);
    U64 startEntry64                            = static_cast<U64>(startEntry);
    U64 szDesc                                  = m_descIncSz;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuSrcHandle    = { m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + startEntry64 * szDesc };
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { 0 };

    if (m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
        cpuDstHandle = { m_pGPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + endEntry64 * szDesc };
    } else {
        cpuDstHandle = { m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + endEntry64 * szDesc };
    }

    pDevice->CopyDescriptorsSimple(totalEntries,
            cpuSrcHandle, cpuDstHandle, m_descHeapType);

    return R_RESULT_OK;
}


D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::createRenderTargetView(
    U32 entryOffset, const D3D12_RENDER_TARGET_VIEW_DESC& desc,
    ID3D12Resource* pResource)
{
    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    ID3D12Device* pDevice   = m_pDevice->get();
    U64 offset64            = static_cast<U64>(entryOffset);
    U64 incSz64             = static_cast<U64>(m_descIncSz);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle   = m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { cpuBaseHandle.ptr + offset64 * incSz64 };
    
    pDevice->CreateRenderTargetView(pResource, &desc, cpuDstHandle);

    return cpuDstHandle;
}


D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::createDepthStencilView(
    U32 entryOffset, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc,
    ID3D12Resource* pResource)
{
    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    ID3D12Device* pDevice   = m_pDevice->get();
    U64 offset64            = static_cast<U64>(entryOffset);
    U64 incSz64             = static_cast<U64>(m_descIncSz);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle = m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle = { cpuBaseHandle.ptr + offset64 * incSz64 };
    
    pDevice->CreateDepthStencilView(pResource, &desc, cpuDstHandle);

    return cpuDstHandle;
}


D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::createShaderResourceView(
    U32 entryOffset, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
    ID3D12Resource* pResource)
{
    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    ID3D12Device* pDevice   = m_pDevice->get();
    U64 offset64            = static_cast<U64>(entryOffset);
    U64 incSz64             = static_cast<U64>(m_descIncSz);
    
    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle   = m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { cpuBaseHandle.ptr + offset64 * incSz64 };
    
    pDevice->CreateShaderResourceView(pResource, &desc, cpuDstHandle);
    
    return cpuDstHandle;
}


D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::createUnorderedAccessView(
    U32 entryOffset, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
    ID3D12Resource* pResource)
{
    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ID3D12Device* pDevice   = m_pDevice->get();
    U64 offset64            = static_cast<U64>(entryOffset);
    U64 incSz64             = static_cast<U64>(m_descIncSz);
    
    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle   = m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { cpuBaseHandle.ptr + offset64 * incSz64 };
    
    pDevice->CreateUnorderedAccessView(pResource, nullptr, &desc, cpuDstHandle);

    return cpuDstHandle;
}


D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::getGPUHandle(U32 entryOffset)
{
    R_ASSERT(m_descHeapType != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && m_descHeapType != D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    D3D12_GPU_DESCRIPTOR_HANDLE baseHandle  = m_pGPUDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    D3D12_GPU_VIRTUAL_ADDRESS baseAddress   = baseHandle.ptr;
    U64 offset64                            = static_cast<U64>(entryOffset);
    U64 incSz64                             = static_cast<U64>(m_descIncSz);

    // If we just want the base handle, return it.
    if (entryOffset == 0) {
        return baseHandle;
    }

    D3D12_GPU_VIRTUAL_ADDRESS address   = baseAddress + offset64 * incSz64;  
    D3D12_GPU_DESCRIPTOR_HANDLE handle  = { address };

    return handle;
}


D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::createSampler(U32 entryOffset,
    const D3D12_SAMPLER_DESC& desc)
{
    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    ID3D12Device* pDevice   = m_pDevice->get();
    U64 offset64            = static_cast<U64>(entryOffset);
    U64 incSz64             = static_cast<U64>(m_descIncSz);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle   = m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { cpuBaseHandle.ptr + offset64 * incSz64 };

    pDevice->CreateSampler(&desc, cpuDstHandle);

    return cpuDstHandle;
}
} // Recluse