//
#pragma once 

#include "D3D12Commons.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"

#include <vector>

namespace Recluse {


class D3D12Context;


class D3D12Adapter : public GraphicsAdapter {
public:
    static std::vector<IDXGIAdapter*> getAdapters(D3D12Context* pContext);

    ErrType getAdapterInfo(AdapterInfo* out) const override;
    
    IDXGIAdapter* get() const { return m_pAdapter; }

    D3D12Context* getContext() const { return m_pContext; }

    void destroy() { }

private:

    D3D12Adapter(IDXGIAdapter* adapter = NULL);

    IDXGIAdapter* m_pAdapter;
    D3D12Context* m_pContext;

    friend class D3D12Context;
};
} // Recluse