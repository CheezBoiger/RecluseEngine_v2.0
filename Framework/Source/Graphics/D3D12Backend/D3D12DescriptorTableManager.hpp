//
#pragma once

#include "Recluse/Types.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Graphics/DescriptorSet.hpp"


namespace Recluse {


class D3D12Device;

class D3D12DescriptorHeap 
{
public:

    D3D12DescriptorHeap(D3D12Device* pDevice);

    ~D3D12DescriptorHeap() {
    }

    ErrType initialize(D3D12_DESCRIPTOR_HEAP_TYPE type, U32 entries);
    ErrType destroy();

    ErrType update(U32 startEntry, U32 endEntry);

    UINT getMaxEntries() const { return m_maxEntries; }

    D3D12_CPU_DESCRIPTOR_HANDLE createRenderTargetView
                                    (
                                        U32 entryOffset, 
                                        const D3D12_RENDER_TARGET_VIEW_DESC& desc, 
                                        ID3D12Resource* pResource
                                    );

    D3D12_CPU_DESCRIPTOR_HANDLE createDepthStencilView
                                    (
                                        U32 entryOffset, 
                                        const D3D12_DEPTH_STENCIL_VIEW_DESC& desc,
                                        ID3D12Resource* pResource
                                    );

    D3D12_CPU_DESCRIPTOR_HANDLE createShaderResourceView
                                    (
                                        U32 entryOffset, 
                                        const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
                                        ID3D12Resource* pResource
                                    );

    D3D12_CPU_DESCRIPTOR_HANDLE createUnorderedAccessView
                                    (
                                        U32 entryOffset, 
                                        const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
                                        ID3D12Resource* pResource
                                    );

    D3D12_CPU_DESCRIPTOR_HANDLE createConstantBufferView
                                    (
                                        U32 entryOffset, 
                                        const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc,
                                        ID3D12Resource* pResource
                                    );

    D3D12_CPU_DESCRIPTOR_HANDLE createSampler
                                    (
                                        U32 entryOffset, 
                                        const D3D12_SAMPLER_DESC& desc
                                    );

    D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(U32 entryOffset = 0u);

private:
    D3D12Device* m_pDevice;
    ID3D12DescriptorHeap* m_pCPUDescriptorHeap;
    ID3D12DescriptorHeap* m_pGPUDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_descHeapType;
    UINT                        m_descIncSz;
    UINT                        m_maxEntries;
};


struct DescriptorTable 
{
    U64 offset;
    U64 entries;
};


class D3D12DescriptorSetLayout : public DescriptorSetLayout 
{
public:
    
    ID3D12RootSignature* get() { return m_pRootSignature; }

private:
    ID3D12RootSignature* m_pRootSignature;
};


class D3D12DescriptorSet : public DescriptorSet 
{
public:

private:
    D3D12DescriptorHeap*  m_pManagement;
    std::vector<DescriptorTable>    m_tables;
};


class DescriptorHeapManager 
{
public:

    D3D12_CPU_DESCRIPTOR_HANDLE createSampler(const D3D12_SAMPLER_DESC& desc) 
    {
        const UINT maxEntries = m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].pHeap->getMaxEntries();
        const UINT currOffset = m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].currentOffset;

        m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].currentOffset += 
            (currOffset < maxEntries) ? 1 : 0;

        return m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].pHeap->createSampler
                                                                    (
                                                                        m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].currentOffset, 
                                                                        desc
                                                                    );
    }

    D3D12_CPU_DESCRIPTOR_HANDLE createRtv(const D3D12_RENDER_TARGET_VIEW_DESC& desc, ID3D12Resource* pRtvResource)
    {
        const UINT maxEntries = m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].pHeap->getMaxEntries();
        const UINT currOffset = m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].currentOffset;

        m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].currentOffset +=
            (currOffset < maxEntries) ? 1 : 0;

        return m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].pHeap->createRenderTargetView
                                                                (
                                                                    m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].currentOffset,
                                                                    desc, pRtvResource
                                                                );
    }

    D3D12_CPU_DESCRIPTOR_HANDLE createDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, ID3D12Resource* pDsvResource)
    {
        return {0};
    }

    D3D12_CPU_DESCRIPTOR_HANDLE createUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* pUavResource)
    {
        return {0};
    }

    D3D12_CPU_DESCRIPTOR_HANDLE createSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* pSrvResource)
    {
        return {0};
    }
    
private:

    struct HeapMetaData 
    {
        D3D12DescriptorHeap*    pHeap;
        UINT64                  currentOffset;
    };

    HeapMetaData m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    
};
} // Recluse 