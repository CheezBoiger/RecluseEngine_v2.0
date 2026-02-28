// 
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "D3D12Commons.hpp"
#include "D3D12Allocator.hpp"

namespace Recluse {
namespace D3D12 {

class D3D12Device;
typedef U32 ResourceTransitionFlags;

class D3D12Resource : public GraphicsResource 
{
public:

    GraphicsAPI getApi() const { return GraphicsApi_Direct3D12; }
    
    D3D12Resource(D3D12Device* pDevice = nullptr, ID3D12Resource* pResource = nullptr, ResourceState initState = ResourceState_Common)
        : GraphicsResource()
        , m_memObj({})
        , m_isCommitted(false)
        , m_shouldDiscard(false)
        , m_totalSubresources(0)
        , m_pDevice(pDevice)
    {
        m_memObj.pResource = pResource; 
        setCurrentResourceState(initState);
        if (pResource)
        {
            D3D12_RESOURCE_DESC desc = pResource->GetDesc();
            m_totalSubresources = desc.MipLevels * desc.DepthOrArraySize;
        }
    }

    // Initialize the resource.
    ResultCode                 initialize
                                (
                                    // The logical device to use and manage our resource.
                                    D3D12Device* pDevice, 
                                    // resource description.
                                    const GraphicsResourceDescription& desc, 
                                    // the initial state of the resource.
                                    ResourceState initialState,
                                    // Graphics clear color
                                    GraphicsClearColor* clearColor,
                                    // Making this resource committed, we will not be sub allocating from a fixed
                                    // memory heap. Instead, we will allocate a separate gpu heap, and allocate there.
                                    Bool makeCommitted = false
                                );

    // Destroy the resource.
    ResultCode                  destroy(Bool immediate = false);

    // Get the memory object.
    ID3D12Resource*             get() {         return m_memObj.pResource; }
    const ID3D12Resource*       get() const {   return m_memObj.pResource; }

    // Is the resource committed.
    Bool                        isCommitted() const { return m_isCommitted; }
    Bool                        shouldDiscard() const { return m_shouldDiscard; }

    // Obtain struct that is used to make the actual transition on the GPU.
    D3D12_RESOURCE_BARRIER      transition(U32 subresource, ResourceState newState);
    void                        finalizeTransition(ResourceState newState) { setCurrentResourceState(newState); } 

    // Get the resource virtual address if it was created on GPU memory.
    D3D12_GPU_VIRTUAL_ADDRESS   getVirtualAddress() const { return (m_memObj.usage == ResourceMemoryUsage_GpuOnly) ? m_memObj.pResource->GetGPUVirtualAddress() : 0ULL; }

    ResultCode                  map(void** pMappedMemory, MapRange* pReadRange) override;
    ResultCode                  unmap(MapRange* pWriteRange) override;

    ResourceId                  getId() const { return m_id; }
    U16                         getTotalSubResources() const { return m_totalSubresources; }

    ResourceView                asView(const ResourceViewDescription& description) override;

    void                        generateId() override;
    ResourceView                asCbv(U32 offsetBytes, U32 sizeBytes) override;

    // Discard the resource if needed. This tells the gpu to not care about the previous contents.
    void                        discard(ID3D12GraphicsCommandList* commandList, const D3D12_DISCARD_REGION* discardRegion);

private:
    Bool                        isSupportedTransitionState(ResourceState state);
    HRESULT                     createAsCommitted(ID3D12Device* device, const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE* clearValue);

    D3D12MemoryObject           m_memObj;

    Bool                        m_isCommitted : 1;
    Bool                        m_shouldDiscard : 1;

    U16                         m_totalSubresources;
    ResourceTransitionFlags     m_allowedTransitionStates;
    D3D12Device*                m_pDevice;
    ResourceId                  m_id;
    std::map<Hash64, ResourceView>                  m_viewMap;
    std::map<Hash64, D3D12_CPU_DESCRIPTOR_HANDLE>   m_cbvMap;
};


D3D12Resource* makeResource(D3D12Device* pDevice, const GraphicsResourceDescription& description, ResourceState initialState, GraphicsClearColor* clearColor);
ResultCode     releaseResource(D3D12Resource* pResource, Bool immediate);
} // D3D12
} // Recluse