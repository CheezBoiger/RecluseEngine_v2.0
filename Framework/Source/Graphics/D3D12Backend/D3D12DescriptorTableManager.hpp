//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Graphics/DescriptorSet.hpp"

#include <array>
#include <vector>


namespace Recluse {


class D3D12Device;
class DescriptorHeapAllocation;


enum GpuHeapType
{
    GpuHeapType_CbvSrvUav,
    GpuHeapType_Sampler,

    GpuHeapType_DescriptorHeapCount,
    GpuHeapType_Unknown = (GpuHeapType_DescriptorHeapCount + 1)
};


enum CpuHeapType
{
    CpuHeapType_Rtv,
    CpuHeapType_Dsv,
    CpuHeapType_CbvSrvUav,
    CpuHeapType_Sampler,

    CpuHeapType_DescriptorHeapCount,
    CpuHeapType_Unknown = (CpuHeapType_DescriptorHeapCount + 1)
};


// Descriptor heap handle, which holds onto several descriptor heaps available for allocation/freeing.
// Each descriptor allocation needs to be reallocated every new frame, as old descriptors will be freed.
class DescriptorHeap
{
public:
    // Invalid GPU address handle.
    static const D3D12_GPU_DESCRIPTOR_HANDLE invalidGpuAddress;
    // Invalid CPU address handle.
    static const D3D12_CPU_DESCRIPTOR_HANDLE invalidCpuAddress;

    DescriptorHeap();
    virtual ~DescriptorHeap() { }

    ResultCode                          initialize(ID3D12Device* pDevice, U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
    ResultCode                          release(ID3D12Device* pDevice);

    virtual DescriptorHeapAllocation    allocate(U32 numDescriptors) = 0;
    virtual void                        free(const DescriptorHeapAllocation& allocation) = 0;
    virtual void                        reset();

    // Resize the descriptor heap instance, to support more or less descriptors at a time.
    void                                resize(U32 descriptorCount);

    // Get the descriptor heap description.
    D3D12_DESCRIPTOR_HEAP_DESC          getDesc() const { return m_pHeap->GetDesc(); }
    // Get the native descriptor heap handle.
    ID3D12DescriptorHeap*               getNative() { return m_pHeap; }

    // Check if the descriptor heap is valid.
    Bool                                isValid() const { return (m_pHeap != nullptr); }

    // Is there space in this descriptor heap?
    Bool                                hasAvailableSpace() const { return (m_currentTotalEntries < m_heapDesc.NumDescriptors); }

    Bool                                isShaderVisible() const { return (m_heapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE); }

    D3D12_CPU_DESCRIPTOR_HANDLE         getBaseCpuHandle() const { return m_baseCpuHandle; }

    D3D12_GPU_DESCRIPTOR_HANDLE         getBaseGpuHandle() const { return m_baseGpuHandle; }

    U32                                 getTotalDescriptorCount() { return m_heapDesc.NumDescriptors; }

protected:

    virtual D3D12_DESCRIPTOR_HEAP_DESC makeDescriptorHeapDescription(U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = { };
        desc.NumDescriptors = numDescriptors;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = nodeMask;
        desc.Type = type;
        return desc;
    }

    virtual SmartPtr<Allocator> makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes);

    ID3D12DescriptorHeap*                   m_pHeap;
    std::vector<DescriptorHeapAllocation>   m_freeAllocations;
    U32                                     m_currentTotalEntries;
    U32                                     m_descriptorSize;
    U32                                     m_heapId;
    D3D12_DESCRIPTOR_HEAP_DESC              m_heapDesc;
    SmartPtr<Allocator>                     m_allocator;
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

    // Get total number of descriptors.
    U32                         getTotalDescriptors() const { return m_totalHandles; }

    // Get the gpu descriptor handle based on the entryOffset(index).
    D3D12_GPU_DESCRIPTOR_HANDLE getGpuDescriptor(U32 entryOffset = 0u) const;

    // Get the cpu descriptor handle based on the entryOffset(index).
    D3D12_CPU_DESCRIPTOR_HANDLE getCpuDescriptor(U32 entryOffset = 0u) const;
    
    // Grab the native descriptor heap.
    ID3D12DescriptorHeap*       getNativeDescriptorHeap() { return m_pDescriptorHeap->getNative(); }

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


class CpuDescriptorHeap : public DescriptorHeap
{
public:
    virtual DescriptorHeapAllocation    allocate(U32 numDescriptors) override;
    virtual void                        free(const DescriptorHeapAllocation& allocation) override { }

};


// Because the rendering gpu can only bind a limited set of descriptor heaps per frame, along with a limited number to create due to the 
// memory size of about 96 MB, we should keep one set of gpu descriptor heaps per frame.
class GpuDescriptorHeap : public DescriptorHeap
{
public:
    virtual DescriptorHeapAllocation    allocate(U32 numDescriptors) override;
    virtual void                        free(const DescriptorHeapAllocation& allocation) override { }
    virtual SmartPtr<Allocator>         makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes) override;

protected:
    D3D12_DESCRIPTOR_HEAP_DESC          makeDescriptorHeapDescription(U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type) override
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = DescriptorHeap::makeDescriptorHeapDescription(nodeMask, numDescriptors, type);
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        return desc;
    }
};


enum DescriptorHeapUpdateFlag
{
    DescriptorHeapUpdateFlag_None,
    DescriptorHeapUpdateFlag_Reset = (1 << 0)
};

typedef U32 DescriptorHeapUpdateFlags;


class DescriptorHeapInstance
{
public:
    // Allocate a table of descriptors. Uses device only if we run out of descriptor space... This shouldn't usually happen unless
    // we are overflowing with so many descriptors.
    DescriptorHeapAllocation        allocate(ID3D12Device* pDevice, U32 numberDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);

    // Free an allocation.
    void                            free(const DescriptorHeapAllocation& descriptorAllocation);

    void                            reset();

    // Upload cpu handles to the gpu descriptor heaps.
    void                            upload(ID3D12Device* pDevice);

    // Update this instance.
    void                            update(U32 index, DescriptorHeapUpdateFlags updateFlags);

private:
    std::array<CpuDescriptorHeap, CpuHeapType_DescriptorHeapCount> m_cpuHeap;
    std::array<GpuDescriptorHeap, GpuHeapType_DescriptorHeapCount> m_gpuHeap;
};


class DescriptorHeapAllocationManager 
{
public:
    struct DescriptorCoreSize
    {
        F32 rtvDescriptorCountFactor        = 1.0f;
        F32 dsvDescriptorCountFactor        = 1.0f;
        F32 cbvSrvUavDescriptorCountFactor  = 1.0f;
        F32 samplerDescriptorCountFactor    = 1.0f;
    };

    // Chunk size of each descriptor heap. Maxed out size will likely force a new descriptor heap size creation
    // on next frame.
    static const F32                kNumDescriptorsPageSize;
    static const F32                kNumSamplerDescriptorsPageSize;

    ResultCode                      initialize(ID3D12Device* pDevice, const DescriptorCoreSize& descriptorSizes, U32 bufferCount);
    ResultCode                      release(ID3D12Device* pDevice);

    void                            resize(U32 bufferIndex);

    DescriptorHeapInstance*         getInstance(U32 index)
    {
        return &m_descriptorHeapInstances[index];
    }

private:
    std::vector<DescriptorHeapInstance> m_descriptorHeapInstances;
};


namespace Binder {


ID3D12RootSignature* makeRootSignature();
} // Descriptors
} // Recluse 