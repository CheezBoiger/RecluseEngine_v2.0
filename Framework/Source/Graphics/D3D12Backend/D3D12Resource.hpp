// 
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "D3D12Commons.hpp"

namespace Recluse {


class D3D12Resource : public GraphicsResource {
public:
    
    D3D12Resource(GraphicsResourceDescription& desc)
        : GraphicsResource(desc)
        , m_pResource(nullptr) { }

    ErrType initialize();
    ErrType destroy();

private:
    ID3D12Resource* m_pResource;
};
} // Recluse