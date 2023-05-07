// 
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "D3D12Commons.hpp"
#include "D3D12Allocator.hpp"

namespace Recluse {

class D3D12Device;
typedef U32 ResourceTransitionFlags;

class D3D12Resource : public GraphicsResource 
{
public:
    
    D3D12Resource(GraphicsResourceDescription& desc)
        : GraphicsResource(desc)
        , m_memObj({})
        , m_isCommitted(false) { }

    // Initialize the resource.
    ResultCode                 initialize
                                (
                                    // The logical device to use and manage our resource.
                                    D3D12Device* pDevice, 
                                    // resource description.
                                    const GraphicsResourceDescription& desc, 
                                    // the initial state of the resource.
                                    ResourceState initialState, 
                                    // Making this resource committed, we will not be sub allocating from a fixed
                                    // memory heap. Instead, we will allocate a separate gpu heap, and allocate there.
                                    Bool makeCommitted = false
                                );

    // Destroy the resource.
    ResultCode                 destroy();

    // Get the memory object.
    ID3D12Resource*         get() {         return m_memObj.pResource; }
    const ID3D12Resource*   get() const {   return m_memObj.pResource; }

    // Is the resource committed.
    Bool                    isCommitted() const { return m_isCommitted; }

    // Obtain struct that is used to make the actual transition on the GPU.
    D3D12_RESOURCE_BARRIER  transition(ResourceState newState);

    // Get the resource virtual address if it was created on GPU memory.
    D3D12_GPU_VIRTUAL_ADDRESS getVirtualAddress() const { return (m_memObj.usage == ResourceMemoryUsage_GpuOnly) ? m_memObj.pResource->GetGPUVirtualAddress() : 0ULL; }

    ResultCode map(void** pMappedMemory, MapRange* pReadRange) override;
    ResultCode unmap(MapRange* pWriteRange) override;

private:
    Bool                    isSupportedTransitionState(ResourceState state);

    D3D12MemoryObject       m_memObj;
    Bool                    m_isCommitted;
    ResourceTransitionFlags m_allowedTransitionStates;
};
} // Recluse