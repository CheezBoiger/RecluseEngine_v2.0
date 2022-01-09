// 
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "D3D12Commons.hpp"
#include "D3D12Allocator.hpp"

namespace Recluse {

class D3D12Device;

class D3D12Resource : public GraphicsResource 
{
public:
    
    D3D12Resource(GraphicsResourceDescription& desc)
        : GraphicsResource(desc)
        , m_memObj({})
        , m_isCommitted(false) { }

    ErrType                 initialize
                                (
                                    D3D12Device* pDevice, 
                                    const GraphicsResourceDescription& desc, 
                                    ResourceState initialState, 
                                    Bool isPixelShader, 
                                    Bool makeCommitted = false
                                );

    ErrType                 destroy();
    ID3D12Resource*         get() { return m_memObj.pResource; }
    Bool                    isCommitted() const { return m_isCommitted; }

    // Obtain struct that is used to make the actual transition on the GPU.
    D3D12_RESOURCE_BARRIER  transition(ResourceState newState, Bool isPixelShaderTransition);

private:
    D3D12MemoryObject   m_memObj;
    Bool                m_isCommitted;
};
} // Recluse