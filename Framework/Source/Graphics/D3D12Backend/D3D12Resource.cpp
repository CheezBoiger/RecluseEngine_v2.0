//
#include "D3D12Resource.hpp"
#include "D3D12Device.hpp"
#include "D3D12Allocator.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


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
    ID3D12Device* device                    = pDevice->get();
    D3D12_RESOURCE_DESC d3d12desc           = { };
    D3D12_RESOURCE_STATES state             = getNativeResourceState(initialState);
    D3D12_CLEAR_VALUE optimizedClearValue   = { };
    D3D12ResourceAllocationManager* pAllocator = pDevice->resourceAllocationManager();
    HRESULT sResult                         = S_OK;

    d3d12desc.Dimension         = getDimension(desc.dimension);
    d3d12desc.DepthOrArraySize  = desc.depthOrArraySize;
    d3d12desc.Width             = desc.width;
    d3d12desc.Height            = desc.height;
    d3d12desc.MipLevels         = desc.mipLevels;
    d3d12desc.Format            = Dxgi::getNativeFormat(desc.format);
    d3d12desc.Alignment         = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

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
            &optimizedClearValue, __uuidof(ID3D12Resource), (void**)&m_memObj.pResource);
    } 
    else 
    {   
        R_ASSERT_FORMAT(pAllocator, "No allocator exists for the given dimension! Is the resource unknown?");

        ResultCode result  = pAllocator->allocate(&m_memObj, d3d12desc, desc.memoryUsage, state);

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
                &optimizedClearValue, __uuidof(ID3D12Resource), (void**)&m_memObj.pResource);
        }
    }

    if (FAILED(sResult)) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create resource!");

        return RecluseResult_Failed;
    }

    m_isCommitted               = makeCommitted;
    m_allowedTransitionStates   = desc.usage;

    setCurrentResourceState(initialState);

    return RecluseResult_Ok;
}


ResultCode D3D12Resource::destroy()
{
    if (m_memObj.pResource) 
    {
        if (!m_isCommitted) 
        {
            D3D12ResourcePagedAllocator* pAllocator = nullptr;
            // Get the allocator from the device maybe?
            R_ASSERT(pAllocator != NULL);
            
            ResultCode result = pAllocator->free(&m_memObj);

            if (result != RecluseResult_Ok) 
            {
                R_ERROR("D3D12Resource", "This placed resource failed to free prior to it's destruction!");
                return result;
            }
        }

        m_memObj.pResource->Release();
        m_memObj.pResource = nullptr;

    }

    return RecluseResult_Ok;
}


D3D12_RESOURCE_BARRIER D3D12Resource::transition(ResourceState newState)
{
    D3D12_RESOURCE_BARRIER barrier = { };

    barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource    = m_memObj.pResource;
    barrier.Transition.StateBefore  = getNativeResourceState(getCurrentResourceState());
    barrier.Transition.StateAfter   = getNativeResourceState(newState);
    barrier.Transition.Subresource  = 0u;

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
} // Recluse