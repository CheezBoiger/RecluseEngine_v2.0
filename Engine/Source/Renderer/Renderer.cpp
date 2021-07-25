//

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsContext.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Engine {


void Renderer::initialize(void* windowHandle, const RendererConfigs& configs)
{
    EnableLayerFlags flags  = 0;
    ApplicationInfo info    = { };
    ErrType result          = REC_RESULT_OK;
    m_windowHandle          = windowHandle;

    m_pContext = GraphicsContext::createContext(configs.api);
    
    if (!m_pContext) {

        R_ERR("Renderer", "Failed to create graphics context, aborting...");

        return;
    }

    info.engineName     = RECLUSE_ENGINE_NAME_STRING;
    info.engineMajor    = RECLUSE_ENGINE_VERSION_MAJOR;
    info.engineMinor    = RECLUSE_ENGINE_VERSION_MINOR;
    info.enginePatch    = RECLUSE_ENGINE_VERSION_PATCH;
    info.appName        = "NoName";

    result = m_pContext->initialize(info, flags);

    if (result != REC_RESULT_OK) {

        R_ERR("Renderer", "Failed to initialize context!");        

    }

    std::vector<GraphicsAdapter*> adapters = m_pContext->getGraphicsAdapters();
    
    determineAdapter(adapters);

    createDevice();
}


void Renderer::cleanUp()
{
}


void Renderer::present()
{
}


void Renderer::render()
{
}


void Renderer::determineAdapter(std::vector<GraphicsAdapter*>& adapters)
{
    for (U32 i = 0; i < adapters.size(); ++i) {
        AdapterInfo info            = { };
        AdapterLimits limits        = { };
        GraphicsAdapter* pAdapter   = adapters[i];
        ErrType result              = REC_RESULT_OK;

        result = pAdapter->getAdapterInfo(&info);
    
        if (result != REC_RESULT_OK) {
    
            R_ERR("Renderer", "Failed to query adapter info.");
        
        }

        R_DEBUG("Adapter Name: %s\n\t\tVendor: %s", info.deviceName, info.vendorName);

        // Just really quickly, pick up the first adapter.
        m_pAdapter = pAdapter;
        break;
    }
}


void Renderer::createDevice()
{
    DeviceCreateInfo info   = { };
    info.buffering          = 2;
    info.winHandle          = m_windowHandle;
    ErrType result          = REC_RESULT_OK;
    
    result = m_pAdapter->createDevice(info, &m_pDevice);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Renderer", "Failed to create device!");
        
    }
}
} // Engine
} // Recluse