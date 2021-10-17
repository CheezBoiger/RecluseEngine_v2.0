// 
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "D3D12Commons.hpp"

namespace Recluse {

class D3D12Device;

class D3D12Resource : public GraphicsResource {
public:
    
    D3D12Resource(GraphicsResourceDescription& desc)
        : GraphicsResource(desc)
        , m_pResource(nullptr)
        , m_isCommitted(false) { }

    ErrType initialize(D3D12Device* pDevice, const GraphicsResourceDescription& desc, 
        Bool makeCommitted = false);

    ErrType destroy();

    ID3D12Resource* get() { return m_pResource; }

    Bool isCommitted() const { return m_isCommitted; }

private:
    ID3D12Resource* m_pResource;
    Bool            m_isCommitted;
};
} // Recluse