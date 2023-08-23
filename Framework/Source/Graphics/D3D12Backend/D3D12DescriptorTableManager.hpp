//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Graphics/DescriptorSet.hpp"

#include <array>
#include <vector>
#include <queue>


namespace Recluse {


class D3D12Device;


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


struct DescriptorTable
{
    // Invalid GPU address handle.
    static const D3D12_GPU_DESCRIPTOR_HANDLE invalidGpuAddress;
    // Invalid CPU address handle.
    static const D3D12_CPU_DESCRIPTOR_HANDLE invalidCpuAddress;

    U32                         numberDescriptors;
    U32                         descriptorAtomSize;
    DescriptorTable
        (
            U32 atomSize,
            U32 numberDescriptors = 0
        )
    : numberDescriptors(numberDescriptors) 
    , descriptorAtomSize(atomSize)
    { }
};


struct CpuDescriptorTable : public DescriptorTable
{
    D3D12_CPU_DESCRIPTOR_HANDLE baseCpuDescriptorHandle;

    CpuDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = DescriptorTable::invalidCpuAddress, U32 numberDescriptors = 0)
        : baseCpuDescriptorHandle(cpuHandle)
        , DescriptorTable(numberDescriptors) { }
    D3D12_CPU_DESCRIPTOR_HANDLE getAddress(U32 idx) const { return { baseCpuDescriptorHandle.ptr + idx * descriptorAtomSize }; }
    void invalidate() { baseCpuDescriptorHandle = DescriptorTable::invalidCpuAddress; }
};


struct ShaderVisibleDescriptorTable : public DescriptorTable
{
    D3D12_GPU_DESCRIPTOR_HANDLE baseGpuDescriptorHandle;

    ShaderVisibleDescriptorTable(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = DescriptorTable::invalidGpuAddress, U32 numberDescriptors = 0)
        : baseGpuDescriptorHandle(gpuHandle)
        , DescriptorTable(numberDescriptors) { }
    D3D12_GPU_DESCRIPTOR_HANDLE getAddress(U32 idx) const { return { baseGpuDescriptorHandle.ptr + idx * descriptorAtomSize }; }
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

    ResultCode                              initialize(ID3D12Device* pDevice, U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
    ResultCode                              release();

    virtual CpuDescriptorTable              allocate(U32 numDescriptors) { return CpuDescriptorTable(); }
    virtual void                            free(const CpuDescriptorTable& descriptorTable) { }
    virtual void                            reset();

    // Resize the descriptor heap instance, to support more or less descriptors at a time.
    void                                    resize(U32 descriptorCount);

    // Check if the heap contains this descriptor handle.
    Bool                                    contains(D3D12_CPU_DESCRIPTOR_HANDLE handle);

    // Get the descriptor heap description.
    D3D12_DESCRIPTOR_HEAP_DESC              getDesc() const { return m_pHeap->GetDesc(); }
    // Get the native descriptor heap handle.
    ID3D12DescriptorHeap*                   getNative() { return m_pHeap; }

    // Check if the descriptor heap is valid.
    Bool                                    isValid() const { return (m_pHeap != nullptr); }

    // Is there space in this descriptor heap?
    Bool                                    hasAvailableSpace() const { return (m_currentTotalEntries < m_heapDesc.NumDescriptors); }
    Bool                                    hasAvailableSpaceForRequest(U32 requestedDescriptors) const { return ((m_currentTotalEntries + requestedDescriptors) < m_heapDesc.NumDescriptors); }
    Bool                                    isShaderVisible() const { return (m_heapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE); }

    D3D12_CPU_DESCRIPTOR_HANDLE             getBaseCpuHandle() const { return m_baseCpuHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE             getBaseGpuHandle() const { return m_baseGpuHandle; }

    U32                                     getTotalDescriptorCount() { return m_heapDesc.NumDescriptors; }
    U32                                     getHeapId() const { return m_heapId; }
    U32                                     getAllocatedEntries() const { return m_currentTotalEntries; }
    U32                                     getDescriptorSize() const { return m_descriptorSize; }

protected:

    virtual D3D12_DESCRIPTOR_HEAP_DESC      makeDescriptorHeapDescription(U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = { };
        desc.NumDescriptors = numDescriptors;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = nodeMask;
        desc.Type = type;
        return desc;
    }

    virtual SmartPtr<Allocator>             makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes);

    ID3D12DescriptorHeap*                   m_pHeap;
    std::vector<CpuDescriptorTable>         m_freeAllocations;
    U32                                     m_currentTotalEntries;
    U32                                     m_descriptorSize;
    U32                                     m_heapId;
    D3D12_DESCRIPTOR_HEAP_DESC              m_heapDesc;
    SmartPtr<Allocator>                     m_allocator;
    D3D12_CPU_DESCRIPTOR_HANDLE             m_baseCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE             m_baseGpuHandle;
};


class CpuDescriptorHeap : public DescriptorHeap
{
public:
    virtual CpuDescriptorTable              allocate(U32 numDescriptors) override;
    virtual void                            free(const CpuDescriptorTable& descriptorTable) override;
};


// Because the rendering gpu can only bind a limited set of descriptor heaps, with one type, per frame, along with a limited number to create due to the 
// memory size of about 96 MB, we should keep one set of gpu descriptor heaps per frame.
class ShaderVisibleDescriptorHeap : public DescriptorHeap
{
public:
    virtual SmartPtr<Allocator>             makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes) override;

protected:
    D3D12_DESCRIPTOR_HEAP_DESC              makeDescriptorHeapDescription(U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type) override
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


class ShaderVisibleDescriptorHeapInstance
{
public:
    // Max limitations of hardware is standard to d3d12.
    // These can be found in this link reference.
    //
    // https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
    static const U32                kMaxShaderVisibleHeapDescriptorSize;
    static const U32                kMaxShaderVisibleHeapSamplerDescriptorSize;

    void                            initialize(ID3D12Device* pDevice);
    // Upload cpu handles to the shader visible descriptor heaps. This must be called when 
    // we have already uploaded to cpu staging descriptor heaps, before submitting the commandlist to the gpu.
    ShaderVisibleDescriptorTable    upload(ID3D12Device* pDevice, GpuHeapType type, const CpuDescriptorTable& table);

    // Update this instance.
    void                            update(DescriptorHeapUpdateFlags updateFlags);

    // Obtain a shader visible descriptor heap.
    DescriptorHeap&                 get(GpuHeapType gpuHeapType)
    {
        return m_gpuHeap[gpuHeapType];
    }

    void                            release();

private:
    std::array<ShaderVisibleDescriptorHeap, GpuHeapType_DescriptorHeapCount>    m_gpuHeap;
    std::array<UINT64, GpuHeapType_DescriptorHeapCount>                         m_gpuOffsets;
};


class DescriptorHeapAllocationManager 
{
    static const U32 kMaxedReservedShaderVisibleInstances;
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
    
    DescriptorHeapAllocationManager()
        : m_currentHeapIndex(0)
        , m_currentTableHeapIndex(0)
        , m_pDevice(nullptr)
    { }

    ResultCode                                              initialize(ID3D12Device* pDevice, const DescriptorCoreSize& descriptorSizes, U32 bufferCount);
    ResultCode                                              release();

    void                                                    resizeShaderVisibleHeapInstances(U32 bufferIndex);
    void                                                    resetTableHeaps();

    ShaderVisibleDescriptorHeapInstance*                    getShaderVisibleInstance(U32 index)
    {
        return &m_shaderVisibleDescriptorHeapInstances[index];
    }


    // Table creation, to be used to bind resources in contiguous, or linear, memory.
    CpuDescriptorTable                                      copyDescriptorsToTable(CpuHeapType heapType, D3D12_CPU_DESCRIPTOR_HANDLE* handles, U32 descriptorCount);
    ResultCode                                              freeDescriptorTable(CpuHeapType heapType, const CpuDescriptorTable& table);

    // Individual persistant descriptors creation.
    D3D12_CPU_DESCRIPTOR_HANDLE                             allocateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);

    // Constant buffer views just take the gpu address as the resource.
    D3D12_CPU_DESCRIPTOR_HANDLE                             allocateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
    D3D12_CPU_DESCRIPTOR_HANDLE                             allocateUnorderedAccessView(ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
    D3D12_CPU_DESCRIPTOR_HANDLE                             allocateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
    D3D12_CPU_DESCRIPTOR_HANDLE                             allocateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
    D3D12_CPU_DESCRIPTOR_HANDLE                             nullRtvDescriptor() const { return m_nullRtvDescriptor; }

    ResultCode                                              freeRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);
    ResultCode                                              freeShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);
    ResultCode                                              freeConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);
    ResultCode                                              freeUnorderedAccessView(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);
    ResultCode                                              freeDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

private:
    CpuDescriptorTable                                      internalAllocate(CpuHeapType heapType, U32 numDescriptors);
    CpuDescriptorHeap*                                      createNewCpuDescriptorTableHeap(CpuHeapType type);
    CpuDescriptorHeap*                                      getCurrentTableHeap(CpuHeapType type, U32 requestCount);
    CpuDescriptorHeap*                                      getCurrentHeap(CpuHeapType type, U32 requestCount);
    CpuDescriptorHeap*                                      createNewCpuDescriptorHeap(CpuHeapType type);
    ResultCode                                              internalFree(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, CpuHeapType heapType);

    std::vector<ShaderVisibleDescriptorHeapInstance>        m_shaderVisibleDescriptorHeapInstances;

    // Cpu persistant descriptor heaps, these will hold onto our views, and will then be stored 
    std::map<CpuHeapType, std::vector<CpuDescriptorHeap>>   m_cpuDescriptorHeaps;
    // Temporary descriptor heaps in the form of tables.
    std::map<CpuHeapType, std::vector<CpuDescriptorHeap>>   m_cpuDescriptorTableHeaps;
    D3D12_CPU_DESCRIPTOR_HANDLE                             m_nullRtvDescriptor;
    ID3D12Device*                                           m_pDevice;
    U32                                                     m_currentTableHeapIndex;
    U32                                                     m_currentHeapIndex;
};
} // Recluse 