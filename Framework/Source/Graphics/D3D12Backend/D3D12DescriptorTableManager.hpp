//
#pragma once

#include "Recluse/Types.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Graphics/DescriptorSet.hpp"


namespace Recluse {


class D3D12Device;

class D3D12DescriptorHeapManagement {
public:

    D3D12DescriptorHeapManagement(D3D12Device* pDevice);

    ~D3D12DescriptorHeapManagement() {
    }

    ErrType initialize(D3D12_DESCRIPTOR_HEAP_TYPE type, U32 entries);
    ErrType destroy();

    ErrType update(U32 startEntry, U32 endEntry);

    D3D12_CPU_DESCRIPTOR_HANDLE createRenderTargetView(
        U32 entryOffset, const D3D12_RENDER_TARGET_VIEW_DESC& desc, 
        ID3D12Resource* pResource);

    D3D12_CPU_DESCRIPTOR_HANDLE createDepthStencilView(
        U32 entryOffset, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc,
        ID3D12Resource* pResource);

    D3D12_CPU_DESCRIPTOR_HANDLE createShaderResourceView(
        U32 entryOffset, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
        ID3D12Resource* pResource);

    D3D12_CPU_DESCRIPTOR_HANDLE createUnorderedAccessView(
        U32 entryOffset, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
        ID3D12Resource* pResource);

    D3D12_CPU_DESCRIPTOR_HANDLE createConstantBufferView(
        U32 entryOffset, const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc,
        ID3D12Resource* pResource);

    D3D12_CPU_DESCRIPTOR_HANDLE createSampler(
        U32 entryOffset, const D3D12_SAMPLER_DESC& desc);

    D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(U32 entryOffset = 0u);

private:
    D3D12Device* m_pDevice;
    ID3D12DescriptorHeap* m_pCPUDescriptorHeap;
    ID3D12DescriptorHeap* m_pGPUDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_descHeapType;
    UINT                        m_descIncSz;
};


struct DescriptorTable {
    U64 offset;
    U64 entries;
};


class D3D12DescriptorSetLayout : public DescriptorSetLayout {
public:
    
    ID3D12RootSignature* get() { return m_pRootSignature; }

private:
    ID3D12RootSignature* m_pRootSignature;
};


class D3D12DescriptorSet : public DescriptorSet {
public:

private:
    D3D12DescriptorHeapManagement*  m_pManagement;
    std::vector<DescriptorTable>    m_tables;
};
} // Recluse 