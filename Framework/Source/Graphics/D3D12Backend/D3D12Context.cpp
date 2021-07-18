//
#include "D3D12Context.hpp"
#include "D3D12Adapter.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


void D3D12Context::queryGraphicsAdapters()
{
    std::vector<IDXGIAdapter*> adapters = D3D12Adapter::getAdapters(this);
    m_graphicsAdapters.resize(adapters.size());

    if (adapters.size() > 1) { 

        R_DEBUG(R_CHANNEL_D3D12, "There are %llu D3D12 devices.", adapters.size());

    } else {

        R_DEBUG(R_CHANNEL_D3D12, "There is 1 D3D12 device.");

    }

    for (U32 i = 0; i < m_graphicsAdapters.size(); ++i) {
    
        D3D12Adapter* pAdapter = new D3D12Adapter(adapters[i]);
        pAdapter->m_pContext = this;
        m_graphicsAdapters[i] = pAdapter;
    
    }
}


void D3D12Context::freeGraphicsAdapters()
{
    for (U32 i = 0; i < m_graphicsAdapters.size(); ++i) {
    
        D3D12Adapter* pAdapter = static_cast<D3D12Adapter*>(m_graphicsAdapters[i]);
        pAdapter->destroy();
        delete pAdapter;

    }

    m_graphicsAdapters.clear();
}


ErrType D3D12Context::onInitialize(const ApplicationInfo& appInfo, EnableLayerFlags flags)
{
    R_DEBUG(R_CHANNEL_D3D12, "Initializing D3D12 context...");
    HRESULT result = S_OK;

    result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&m_pFactory);

    if (result != S_OK) {
        return REC_RESULT_FAILED;
    }

    return REC_RESULT_OK;
}


void D3D12Context::onDestroy()
{
    R_DEBUG(R_CHANNEL_D3D12, "Destroying D3D12 context...");

    if (m_pFactory) {

        m_pFactory->Release();
        m_pFactory = nullptr;

    }

    R_DEBUG(R_CHANNEL_D3D12, "Successfully destroyed context!");
}
} // Recluse