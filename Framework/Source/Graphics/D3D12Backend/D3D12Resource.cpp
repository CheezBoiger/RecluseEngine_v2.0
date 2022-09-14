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
        case RESOURCE_DIMENSION_BUFFER: return D3D12_RESOURCE_DIMENSION_BUFFER;
        case RESOURCE_DIMENSION_1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case RESOURCE_DIMENSION_2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case RESOURCE_DIMENSION_3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}


ErrType D3D12Resource::initialize
                            (
                                D3D12Device* pDevice, 
                                const GraphicsResourceDescription& desc, 
                                ResourceState initialState, 
                                Bool isPixelShader, 
                                Bool makeCommitted
                            )
{
    ID3D12Device* device                    = pDevice->get();
    D3D12_RESOURCE_DESC d3d12desc           = { };
    D3D12_RESOURCE_STATES state             = Dxgi::getNativeResourceState(initialState, isPixelShader);
    D3D12_CLEAR_VALUE optimizedClearValue   = { };
    D3D12ResourceAllocator* pAllocator              = nullptr;
    HRESULT sResult                         = S_OK;

    d3d12desc.Dimension         = getDimension(desc.dimension);
    d3d12desc.DepthOrArraySize  = desc.depth;
    d3d12desc.Width             = desc.width;
    d3d12desc.Height            = desc.height;
    d3d12desc.MipLevels         = desc.mipLevels;
    d3d12desc.Format            = Dxgi::getNativeFormat(desc.format);

    if (makeCommitted == true) 
    {
        D3D12_HEAP_PROPERTIES heapProps = { };
        D3D12_HEAP_FLAGS flags          = D3D12_HEAP_FLAG_NONE;

        sResult = device->CreateCommittedResource(&heapProps, flags, &d3d12desc, state, 
            &optimizedClearValue, __uuidof(ID3D12Resource), (void**)&m_memObj.pResource);

    } 
    else 
    {
        if (d3d12desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            pAllocator = pDevice->getBufferAllocator(desc.memoryUsage);
        }
        else if (d3d12desc.Dimension != D3D12_RESOURCE_DIMENSION_UNKNOWN)
        {
            pAllocator = pDevice->getTextureAllocator();
        }

        R_ASSERT_MSG(pAllocator, "No allocator exists for the given dimension! Is the resource unknown?");

        R_NO_IMPL();

        ErrType result  = pAllocator->allocate(device, &m_memObj, d3d12desc, state);

        if (result != R_RESULT_OK)    
        {
            R_ERR(R_CHANNEL_D3D12, "Failed to allocate a D3D12 resource!");
        } 
        else 
        {
            
        }
    }

    if (FAILED(sResult)) 
    {
        R_ERR(R_CHANNEL_D3D12, "Failed to create resource!");

        return R_RESULT_FAILED;
    }

    m_isCommitted   = makeCommitted;
    m_currentState  = initialState;

    return R_RESULT_OK;
}


ErrType D3D12Resource::destroy()
{
    if (m_memObj.pResource) 
    {
        if (!m_isCommitted) 
        {
            D3D12ResourceAllocator* pAllocator = nullptr;
            // Get the allocator from the device maybe?
            R_ASSERT(pAllocator != NULL);
            
            ErrType result = pAllocator->free(&m_memObj);

            if (result != R_RESULT_OK) 
            {
                R_ERR("D3D12Resource", "This placed resource failed to free prior to it's destruction!");
                return result;
            }
        }

        m_memObj.pResource->Release();
        m_memObj.pResource = nullptr;

    }

    return R_RESULT_OK;
}


D3D12_RESOURCE_BARRIER D3D12Resource::transition(ResourceState newState, Bool isPixelShaderTransition)
{
    D3D12_RESOURCE_BARRIER barrier = { };

    barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource    = m_memObj.pResource;
    barrier.Transition.StateBefore  = Dxgi::getNativeResourceState(m_currentState, isPixelShaderTransition);
    barrier.Transition.StateAfter   = Dxgi::getNativeResourceState(newState, isPixelShaderTransition);
    barrier.Transition.Subresource  = 0u;

    return barrier;
}
} // Recluse