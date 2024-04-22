//
#include "D3D12Resource.hpp"
#include "D3D12Device.hpp"
#include "D3D12Instance.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Allocator.hpp"
#include "D3D12ResourceView.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Threading/Threading.hpp"
#include <memory>

namespace Recluse {
namespace D3D12 {

MutexGuard              g_resourceMutex = { };
ResourceId              g_resourceCounter = 0;
std::unordered_map<ResourceId, std::unique_ptr<D3D12Resource>> m_resourceMap;


R_INTERNAL
ResourceId generateResourceId()
{
    ScopedLock _(g_resourceMutex);
    return ++g_resourceCounter;
}


D3D12_RESOURCE_DIMENSION getDimension(ResourceDimension dim)
{
    switch (dim) 
    {
        case ResourceDimension_Buffer:  return D3D12_RESOURCE_DIMENSION_BUFFER;
        case ResourceDimension_1d:      return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case ResourceDimension_2d:      return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case ResourceDimension_3d:      return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:                        return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}


ResultCode D3D12Resource::initialize
                            (
                                D3D12Device* pDevice, 
                                const GraphicsResourceDescription& desc, 
                                ResourceState initialState,
                                Bool makeCommitted
                            )
{
    ID3D12Device* device                        = pDevice->get();
    D3D12_RESOURCE_DESC d3d12desc               = { };
    D3D12_RESOURCE_STATES state                 = getNativeResourceState(initialState);
    D3D12_CLEAR_VALUE optimizedClearValue       = { };
    D3D12ResourceAllocationManager* pAllocator  = pDevice->resourceAllocationManager();
    HRESULT sResult                             = S_OK;

    d3d12desc.Dimension                         = getDimension(desc.dimension);
    d3d12desc.DepthOrArraySize                  = desc.depthOrArraySize;
    d3d12desc.Width                             = desc.width;
    d3d12desc.Height                            = desc.height;
    d3d12desc.MipLevels                         = desc.mipLevels;
    d3d12desc.Format                            = Dxgi::getNativeFormat(desc.format);
    d3d12desc.SampleDesc.Count                  = 1;
    d3d12desc.SampleDesc.Quality                = 0;
    d3d12desc.Alignment                         = 0;
    d3d12desc.Flags                             = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    
    if (d3d12desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        d3d12desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        // We need to align up on constant buffer resources!!
        if (desc.usage & ResourceUsage_ConstantBuffer)
        {
            d3d12desc.Width = align(desc.width, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        }
    }
    else
    {
        // Textures should remain using an optimal layout strategy, which is mainly device dependent. This
        // means the device decides on the best layout for our texture, which should be at best performance.
        // Should a texture need to be rowmajor, will require a rowpitch to read, as layout will require padding 
        // for better reads, but performance can be heavily impacted.
        d3d12desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    }

    if (desc.usage & ResourceUsage_DepthStencil)    d3d12desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if (desc.usage & ResourceUsage_RenderTarget)    d3d12desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (desc.usage & ResourceUsage_UnorderedAccess) d3d12desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (desc.usage & ResourceUsage_ShaderResource)  d3d12desc.Flags &= ~(D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

    D3D12_CLEAR_VALUE* clearValue = nullptr;

    if ((d3d12desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER) && (d3d12desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)))
    {
        optimizedClearValue.Format = d3d12desc.Format;
        clearValue = &optimizedClearValue;
    }

    Bool shouldTransition = false;
    if (desc.memoryUsage == ResourceMemoryUsage_CpuToGpu || desc.memoryUsage == ResourceMemoryUsage_CpuOnly)
    {
        // If the memoryUsage is CpuToGpu or CpuOnly, then d3d12 requires that we are in GENERIC_READ as initial state.
        shouldTransition = !(D3D12_RESOURCE_STATE_GENERIC_READ & state);
        state = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    if (makeCommitted == true) 
    {
        D3D12_HEAP_PROPERTIES heapProps = { };
        D3D12_HEAP_FLAGS flags          = D3D12_HEAP_FLAG_NONE;

        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.CreationNodeMask = 0;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.VisibleNodeMask = 0;

        sResult = device->CreateCommittedResource(&heapProps, flags, &d3d12desc, state, 
            clearValue, __uuidof(ID3D12Resource), (void**)&m_memObj.pResource);
    } 
    else 
    {   
        // We require default placement alignment for our resource.
        d3d12desc.Alignment                         = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        R_ASSERT_FORMAT(pAllocator, "No allocator exists for the given dimension! Is the resource unknown?");

        ResultCode result  = pAllocator->allocate(&m_memObj, d3d12desc, desc.memoryUsage, clearValue, state);

        if (result != RecluseResult_Ok)    
        {
            R_ERROR(R_CHANNEL_D3D12, "Failed to allocate a D3D12 resource!");

            // We will have to do a committed resource allocation instead!
            R_WARN(R_CHANNEL_D3D12, "Failed to perform a sub-allocation for the given resource request, resorting to committed allocation...");
            makeCommitted = true;
            D3D12_HEAP_PROPERTIES heapProps = { };
            D3D12_HEAP_FLAGS flags          = D3D12_HEAP_FLAG_NONE;

            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.CreationNodeMask = 0;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.VisibleNodeMask = 0;

            sResult = device->CreateCommittedResource(&heapProps, flags, &d3d12desc, state, 
                clearValue, __uuidof(ID3D12Resource), (void**)&m_memObj.pResource);
        }
    }

    if (FAILED(sResult)) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create resource!");

        return RecluseResult_Failed;
    }

    m_isCommitted               = makeCommitted;
    m_allowedTransitionStates   = desc.usage;
    m_pDevice                   = pDevice;
    m_totalSubresources         = desc.mipLevels * desc.depthOrArraySize;

    setCurrentResourceState(initialState);
    m_id = generateResourceId();

    // Check for debug marking feature if we need to name these resources.
    D3D12Instance* instance = pDevice->getAdapter()->getInstance();
    R_ASSERT(instance);
    if (desc.name && instance->isLayerFeatureEnabled(LayerFeatureFlag_DebugMarking))
    {
        std::vector<WCHAR> name;
        name.resize(strlen(desc.name) + 1);
        mbstowcs(name.data(), desc.name, name.size());
        m_memObj.pResource->SetName(name.data());
    }

    if (shouldTransition)
    {
        D3D12Queue* pQueue = m_pDevice->getQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        ID3D12GraphicsCommandList* list = pQueue->createOneTimeCommandList(0, m_pDevice->get());
        D3D12_RESOURCE_BARRIER barrier = { };
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = m_memObj.pResource;
        barrier.Transition.StateBefore = state;
        barrier.Transition.StateAfter = getNativeResourceState(getCurrentResourceState());
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        list->ResourceBarrier(1, &barrier);
        list->Close();
        pQueue->endAndSubmitOneTimeCommandList(list);
    }

    return RecluseResult_Ok;
}


ResultCode D3D12Resource::destroy(Bool immediate)
{
    R_ASSERT(m_pDevice != nullptr);

    for (auto view : m_viewMap)
    {
        DescriptorViews::destroyResourceView(m_pDevice, view.second);
    }
    
    for (auto cbv : m_cbvMap)
    {
        DescriptorViews::destroyCbv(m_pDevice, cbv.second);
    }

    if (m_memObj.pResource) 
    {
        if (!m_isCommitted) 
        {
            D3D12ResourceAllocationManager* pAllocator = m_pDevice->resourceAllocationManager();
            // Get the allocator from the device maybe?
            R_ASSERT(pAllocator != NULL);
            
            ResultCode result = pAllocator->free(&m_memObj, immediate);

            if (result != RecluseResult_Ok) 
            {
                R_ERROR("D3D12Resource", "This placed resource failed to free prior to it's destruction!");
                return result;
            }
        }

        m_viewMap.clear();
        m_cbvMap.clear();
        m_memObj.pResource = nullptr;

    }

    return RecluseResult_Ok;
}


D3D12_RESOURCE_BARRIER D3D12Resource::transition(U32 subresource, ResourceState newState)
{
    D3D12_RESOURCE_BARRIER barrier = { };

    barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource    = m_memObj.pResource;
    barrier.Transition.StateBefore  = getNativeResourceState(getCurrentResourceState());
    barrier.Transition.StateAfter   = getNativeResourceState(newState);
    barrier.Transition.Subresource  = subresource;

    return barrier;
}


Bool D3D12Resource::isSupportedTransitionState(ResourceState state)
{
    switch (state)
    {
    case ResourceState_ConstantBuffer:
        return (m_allowedTransitionStates & ResourceUsage_ConstantBuffer);
    case ResourceState_CopyDestination:
        return (m_allowedTransitionStates & ResourceUsage_CopyDestination);
    case ResourceState_CopySource:
        return (m_allowedTransitionStates & ResourceUsage_CopySource);
    case ResourceState_DepthStencilReadOnly:
    case ResourceState_DepthStencilWrite:
        return (m_allowedTransitionStates & ResourceUsage_DepthStencil);
    case ResourceState_UnorderedAccess:
        return (m_allowedTransitionStates & ResourceUsage_UnorderedAccess);
    case ResourceState_VertexBuffer:
        return (m_allowedTransitionStates & ResourceUsage_ConstantBuffer);
    case ResourceState_IndexBuffer:
        return (m_allowedTransitionStates & ResourceUsage_IndexBuffer);
    case ResourceState_IndirectArgs:
        return (m_allowedTransitionStates & ResourceUsage_IndirectBuffer);
    case ResourceState_ShaderResource:
        return (m_allowedTransitionStates & ResourceUsage_ShaderResource);
    }

    return 0;
}


ResourceViewId D3D12Resource::asView(const ResourceViewDescription& description)
{
    ResourceViewId view = 0;
    Hash64 hash = recluseHashFast(&description, sizeof(ResourceViewDescription));
    auto iter = m_viewMap.find(hash);
    if (iter == m_viewMap.end())
    {
        view = DescriptorViews::makeResourceView(m_pDevice, m_memObj.pResource, description);
        m_viewMap.insert(std::make_pair(hash, view));
    }
    else
    {
        view = iter->second;
    }

    return view;
}


D3D12_CPU_DESCRIPTOR_HANDLE D3D12Resource::asCbv(U32 offsetBytes, U32 sizeBytes)
{
    ResourceViewId view = 0;
    Hash64 hash = (((U64)sizeBytes << 32) | (U64)offsetBytes);
    auto iter = m_cbvMap.find(hash);
    if (iter == m_cbvMap.end())
    {   
        D3D12_GPU_VIRTUAL_ADDRESS address = m_memObj.pResource->GetGPUVirtualAddress() + (D3D12_GPU_VIRTUAL_ADDRESS)offsetBytes;
        D3D12_CPU_DESCRIPTOR_HANDLE handle = DescriptorViews::makeCbv(m_pDevice, address, align(sizeBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
        m_cbvMap.insert(std::make_pair(hash, handle));
        return handle;
    }
    else
    {
        return iter->second;
    }
}


void D3D12Resource::generateId()
{
    m_id = generateResourceId();
}


D3D12Resource* makeResource(D3D12Device* pDevice, const GraphicsResourceDescription& description, ResourceState initialState)
{
    R_ASSERT_FORMAT(description.width > 0 && description.height > 0 && description.depthOrArraySize > 0 && description.mipLevels > 0, "Description width/height/arraySize/mipLevels should at least be 1 or greater!");
    std::unique_ptr<D3D12Resource> pResource = std::make_unique<D3D12Resource>();
    ResultCode result = pResource->initialize(pDevice, description, initialState);
    if (result != RecluseResult_Ok)
    {
        pResource.reset();
    }
    else
    {
        ResourceId id = pResource->getId();
        m_resourceMap[id] = std::move(pResource);
        return m_resourceMap[id].get();
    }
    return nullptr;
}


ResultCode releaseResource(D3D12Resource* pResource, Bool immediate)
{
    if (!pResource)
    {
        return RecluseResult_NullPtrExcept;
    }
    auto iter = m_resourceMap.find(pResource->getId());
    if (iter == m_resourceMap.end())
    {
        return RecluseResult_NotFound;
    }
    ResultCode result = pResource->destroy(immediate);
    if (result == RecluseResult_Ok)
    {
        m_resourceMap.erase(iter);
    }
    return RecluseResult_Ok;
}


ResultCode D3D12Resource::map(void** pMappedMemory, MapRange* pReadRange)
{
    // Should ensure this memory is host-visible (cpu visible.)
    R_ASSERT(m_memObj.usage == ResourceMemoryUsage_CpuOnly || m_memObj.usage == ResourceMemoryUsage_CpuToGpu);
    HRESULT result = S_OK;
    if (pReadRange)
    {
        // Api read-write range is a vector, so we need to compensate for this in D3D12.
        D3D12_RANGE readRange = { };
        readRange.Begin = pReadRange->offsetBytes;
        readRange.End = readRange.Begin + pReadRange->sizeBytes;
        result = m_memObj.pResource->Map(0, &readRange, pMappedMemory);
        // Since we are obtaining the buffer ptr location at the start of the resource, 
        // we need to adjust the memory pointer to the beginning offset.
        UPtr memoryPtr =(UPtr)*pMappedMemory;
        memoryPtr = memoryPtr + readRange.Begin;
        *pMappedMemory = (void*)memoryPtr;
    }
    else
    {
        result = m_memObj.pResource->Map(0, nullptr, pMappedMemory);
    }
    return SUCCEEDED(result) ? RecluseResult_Ok : RecluseResult_Failed;
}


ResultCode D3D12Resource::unmap(MapRange* pWriteRange)
{
    HRESULT result = S_OK;
    if (pWriteRange)
    {
        // Api read-write range is a vector, so we need to compensate for this in D3D12.
        D3D12_RANGE range = { };
        range.Begin = pWriteRange->offsetBytes;
        range.End = range.Begin + pWriteRange->sizeBytes;
        m_memObj.pResource->Unmap(0, &range);
    }
    else
    {
        m_memObj.pResource->Unmap(0, nullptr);
    }
    return RecluseResult_Ok;
}
} // D3D12
} // Recluse