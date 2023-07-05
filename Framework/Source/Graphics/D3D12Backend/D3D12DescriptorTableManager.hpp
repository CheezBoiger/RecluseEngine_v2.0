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
    CpuHeapType_CbvSrvUavStaging,
    CpuHeapType_SamplerStaging,

    CpuHeapType_DescriptorHeapCount,
    CpuHeapType_Unknown = (CpuHeapType_DescriptorHeapCount + 1)
};


struct DescriptorTable
{
    // Invalid GPU address handle.
    static const D3D12_GPU_DESCRIPTOR_HANDLE invalidGpuAddress;
    // Invalid CPU address handle.
    static const D3D12_CPU_DESCRIPTOR_HANDLE invalidCpuAddress;

    D3D12_CPU_DESCRIPTOR_HANDLE baseCpuDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE baseGpuDescriptorHandle;
    U32                         numberDescriptors;
    DescriptorTable
        (
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = invalidCpuAddress, 
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = invalidGpuAddress, 
            U32 numberDescriptors = 0
        )
    : baseCpuDescriptorHandle(cpuDescriptorHandle)
    , baseGpuDescriptorHandle(gpuDescriptorHandle)
    , numberDescriptors(numberDescriptors) 
    { }
};


// Descriptor heap handle, which holds onto several descriptor heaps available for allocation/freeing.
// Each descriptor allocation needs to be reallocated every new frame, as old descriptors will be freed.
// The given allocator for this descriptor heap should very much be high performing, otherwise there is a 
// chance we will be bottlenecked on allocations.
class DescriptorHeap
{
public:

    DescriptorHeap();
    virtual ~DescriptorHeap() { }

    ResultCode                          initialize(ID3D12Device* pDevice, U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
    ResultCode                          release(ID3D12Device* pDevice);

    virtual DescriptorTable             allocate(U32 numDescriptors) { return DescriptorTable(); }
    virtual void                        free(const DescriptorTable& descriptorTable) { }
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
    Bool                                hasAvailableSpaceForRequest(U32 requestedDescriptors) const { return ((m_currentTotalEntries + requestedDescriptors) < m_heapDesc.NumDescriptors); }

    Bool                                isShaderVisible() const { return (m_heapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE); }

    D3D12_CPU_DESCRIPTOR_HANDLE         getBaseCpuHandle() const { return m_baseCpuHandle; }

    D3D12_GPU_DESCRIPTOR_HANDLE         getBaseGpuHandle() const { return m_baseGpuHandle; }

    U32                                 getTotalDescriptorCount() { return m_heapDesc.NumDescriptors; }
    U32                                 getHeapId() const { return m_heapId; }
    U32                                 getAllocatedEntries() const { return m_currentTotalEntries; }

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


// Descriptor heap allocation, houses single descriptors and descriptor tables that 
// will contain the necessary addresses needed for binding resources.
// For resources that must be shader visible (cbvs, uavs, srvs, samplers) 
// there is usually a gpu descriptor handle that is mapped along with the allocation.
//
class DescriptorHeapAllocation
{
public:

    DescriptorHeapAllocation
        (
            DescriptorHeap* pDescriptorHeap = nullptr, 
            const DescriptorTable& descriptorTable = { },
            U32 descriptorSize = 0,
            U32 heapId = ~0
        );

    ~DescriptorHeapAllocation() {}

    // Get total number of descriptors.
    U32                         getTotalDescriptors() const { return m_descriptorTable.numberDescriptors; }

    // Get the gpu descriptor handle based on the entryOffset(index).
    D3D12_GPU_DESCRIPTOR_HANDLE getGpuDescriptor(U32 entryOffset = 0u) const;

    // Get the cpu descriptor handle based on the entryOffset(index).
    D3D12_CPU_DESCRIPTOR_HANDLE getCpuDescriptor(U32 entryOffset = 0u) const;

    // Get a copy of the descriptor table.
    DescriptorTable             getDescriptorTableCopy() const { return m_descriptorTable; }
    
    // Grab the native descriptor heap.
    ID3D12DescriptorHeap*       getNativeDescriptorHeap() { return m_pDescriptorHeap->getNative(); }

    // Check if the descriptor heap is shader visible, can be visible to our shaders for binding and 
    // using resources.
    Bool                        isShaderVisible() const { return (m_descriptorTable.baseGpuDescriptorHandle.ptr != 0); }

    // Check if the descriptor heap is valid, meaning if it has been initialized.
    Bool                        isValid() const { return m_descriptorTable.baseCpuDescriptorHandle.ptr != 0; }

    // The actual descriptor atom size provided by the device context (usually from the driver itself.)
    U32                         getDescriptorSize() const { return m_descIncSz; }
    D3D12_DESCRIPTOR_HEAP_TYPE  getDescriptorType() const { return m_pDescriptorHeap->getDesc().Type; }

    // Sets a gpu descriptor handle to this allocation.
    void                        assignGpuDescriptor(D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor) { m_descriptorTable.baseGpuDescriptorHandle = gpuDescriptor; }

private:
    // The descriptor heap.
    DescriptorHeap*             m_pDescriptorHeap;
    U32                         m_descIncSz;
    U32                         m_heapId;
    DescriptorTable             m_descriptorTable;
};


class CpuDescriptorHeap : public DescriptorHeap
{
public:
    virtual DescriptorTable allocate(U32 numDescriptors) override;
    virtual void            free(const DescriptorTable& descriptorTable) override { }

};


// Because the rendering gpu can only bind a limited set of descriptor heaps, with one type, per frame, along with a limited number to create due to the 
// memory size of about 96 MB, we should keep one set of gpu descriptor heaps per frame.
class ShaderVisibleDescriptorHeap : public DescriptorHeap
{
public:
    virtual DescriptorTable             allocate(U32 numDescriptors) override;
    virtual void                        free(const DescriptorTable& descriptorTable) override { }
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
    // we are overflowing with so many descriptors per frame.
    DescriptorHeapAllocation        allocate(ID3D12Device* pDevice, U32 numberDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);

    // Free an allocation.
    void                            free(const DescriptorHeapAllocation& descriptorAllocation);

    void                            reset();

    // Upload cpu handles to the shader visible descriptor heaps. This must be called when 
    // we have already uploaded to cpu staging descriptor heaps, before submitting the commandlist to the gpu.
    void                            upload(ID3D12Device* pDevice);

    // Update this instance.
    void                            update(DescriptorHeapUpdateFlags updateFlags);

    // Obtain a cpu visible descriptor heap.
    DescriptorHeap&                 get(CpuHeapType cpuHeapType)
    {
        return m_cpuHeap[cpuHeapType];
    }

    // Obtain a shader visible descriptor heap.
    DescriptorHeap&                 getShaderVisible(GpuHeapType gpuHeapType)
    {
        return m_gpuHeap[gpuHeapType];
    }

private:
    std::array<CpuDescriptorHeap, CpuHeapType_DescriptorHeapCount> m_cpuHeap;
    std::array<ShaderVisibleDescriptorHeap, GpuHeapType_DescriptorHeapCount> m_gpuHeap;
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