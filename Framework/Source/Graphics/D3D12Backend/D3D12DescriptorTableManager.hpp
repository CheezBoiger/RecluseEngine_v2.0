//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Graphics/DescriptorSet.hpp"


namespace Recluse {


class D3D12Device;
class DescriptorHeapAllocation;


// Descriptor heap handle, which holds onto several descriptor heaps available for allocation/freeing.
//
class DescriptorHeap
{
public:
    // Invalid GPU address handle.
    static const D3D12_GPU_DESCRIPTOR_HANDLE invalidGpuAddress;
    // Invalid CPU address handle.
    static const D3D12_CPU_DESCRIPTOR_HANDLE invalidCpuAddress;

    DescriptorHeap();
    virtual ~DescriptorHeap() { }

    ErrType                             initialize(D3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC& desc);
    ErrType                             destroy(D3D12Device* pDevice);

    virtual DescriptorHeapAllocation    allocate(U32 numDescriptors) = 0;
    virtual void                        free(const DescriptorHeapAllocation& allocation) = 0;

    // Get the descriptor heap description.
    D3D12_DESCRIPTOR_HEAP_DESC          getDesc() const { return m_pHeap->GetDesc(); }
    // Get the native descriptor heap handle.
    ID3D12DescriptorHeap*               getHeap() { return m_pHeap; }

    // Check if the descriptor heap is valid.
    Bool                                isValid() const { return (m_pHeap != nullptr); }

    // Is there space in this descriptor heap?
    Bool                                hasAvailableSpace() const { return (m_currentTotalEntries < m_heapDesc.NumDescriptors); }

    Bool                                isShaderVisible() const { return (m_heapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE); }

    D3D12_CPU_DESCRIPTOR_HANDLE         getBaseCpuHandle() const { return m_baseCpuHandle; }

    D3D12_GPU_DESCRIPTOR_HANDLE         getBaseGpuHandle() const { return m_baseGpuHandle; }

protected:
    ID3D12DescriptorHeap*                   m_pHeap;
    std::vector<DescriptorHeapAllocation>   m_freeAllocations;
    U32                                     m_currentTotalEntries;
    U32                                     m_descriptorSize;
    U32                                     m_heapId;
    D3D12_DESCRIPTOR_HEAP_DESC              m_heapDesc;
    Allocator*                              m_pDescriptorHeapAllocator;
    D3D12_CPU_DESCRIPTOR_HANDLE             m_baseCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE             m_baseGpuHandle;
};


class DescriptorHeapAllocation
{
public:

    DescriptorHeapAllocation
        (
            DescriptorHeap* pDescriptorHeap = nullptr, 
            D3D12_CPU_DESCRIPTOR_HANDLE baseCpuHandle = DescriptorHeap::invalidCpuAddress, 
            D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle = DescriptorHeap::invalidGpuAddress,
            U32 totalHandles = 0,
            U32 descriptorSize = 0,
            U32 heapId = ~0
        );

    ~DescriptorHeapAllocation() {}

    U32                         getTotalHandles() const { return m_totalHandles; }

    // Get the gpu descriptor handle based on the entryOffset(index).
    D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle(U32 entryOffset = 0u) const;

    // Get the cpu descriptor handle based on the entryOffset(index).
    D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(U32 entryOffset = 0u) const;
    
    // Grab the native descriptor heap.
    ID3D12DescriptorHeap*       getNativeDescriptorHeap() { return m_pDescriptorHeap->getHeap(); }

    // Check if the descriptor heap is shader visible, can be visible to our shaders for binding and 
    // using resources.
    Bool                        isShaderVisible() const { return (m_baseGpuDescriptorHandle.ptr != 0); }

    // Check if the descriptor heap is valid, meaning if it has been initialized.
    Bool                        isValid() const { return m_baseCpuDescriptorHandle.ptr != 0; }

    // The actual descriptor atom size provided by the device context (usually from the driver itself.)
    U32                         getDescriptorSize() const { return m_descIncSz; }

private:
    // The descriptor heap.
    DescriptorHeap*             m_pDescriptorHeap;
    U32                         m_descIncSz;
    U32                         m_totalHandles;
    U32                         m_heapId;
    D3D12_CPU_DESCRIPTOR_HANDLE m_baseCpuDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_baseGpuDescriptorHandle;
};


// Multiple Cpu descriptor heaps can exist during the lifetime of the application.
// This is because the rendering gpu has a limited set of descriptor heaps it may be able to bind per frame.
class CpuDescriptorHeap : public DescriptorHeap
{
public:
    virtual DescriptorHeapAllocation    allocate(U32 numDescriptors) override;
    virtual void                        free(const DescriptorHeapAllocation& allocation) override { }
};


// Because the rendering gpu can only bind a limited set of descriptor heaps per frame, along with a limited number to create due to the 
// memory size of about 96 MB, we should keep one set of gpu descriptor heaps. We don't need to create more.
class GpuDescriptorHeap : public DescriptorHeap
{
public:
    virtual DescriptorHeapAllocation    allocate(U32 numDescriptors) override;
    virtual void                        free(const DescriptorHeapAllocation& allocation) override { }
};


// Descriptor heap manager handler. Essentially a helper.
class CpuDescriptorHeapManager
{
public:
    ErrType                     initialize(const D3D12_DESCRIPTOR_HEAP_DESC &desc);
    DescriptorHeapAllocation    allocate(U32 numberDescriptors);
    void                        free(const DescriptorHeapAllocation& alloc);
private:
    std::vector<CpuDescriptorHeap> m_cpuHeaps;
};


class DescriptorHeapAllocationManager 
{
public:
    enum GpuHeapType
    {
        GpuHeapType_CbvSrvUav,
        GpuHeapType_Sampler,
        GpuHeapType_DescriptorHeapCount,
        GpuHeapType_Unknown
    };

    enum CpuHeapType
    {
        CpuHeapType_Rtv,
        CpuHeapType_Dsv,
        CpuHeapType_CbvSrvUav,
        CpuHeapType_Sampler,
        CpuHeapType_DescriptorHeapCount,
        CpuHeapType_Unknown
    };

    struct DescriptorCoreSize
    {
        F32 rtvDescriptorCountFactor = 1.0f;
        F32 dsvDescriptorCountFactor = 1.0f;
        F32 cbvSrvUavDescriptorCountFactor = 1.0f;
        F32 samplerDescriptorCountFactor = 1.0f;
    };

    // Chunk size of each descriptor heap. When a chunk is fully filled with descriptors, we create a new sized chunk based on this
    // size value.
    static const F32 kNumDescriptorsPageSize;

    ErrType                     initialize(ID3D12Device* pDevice, const DescriptorCoreSize& descriptorSizes);
    ErrType                     release(ID3D12Device* pDevice);

    DescriptorHeapAllocation    allocate(U32 numberDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
    void                        free(const DescriptorHeapAllocation& alloc);

    // Upload a section of cpu handles to the gpu descriptor heap.
    void                        upload();

private:

    CpuDescriptorHeapManager        m_cpuDescriptorHeapManagers[CpuHeapType_DescriptorHeapCount];
    GpuDescriptorHeap               m_gpuHeaps[GpuHeapType_DescriptorHeapCount];
};


namespace Binder {


ID3D12RootSignature* makeRootSignature();
} // Descriptors
} // Recluse 