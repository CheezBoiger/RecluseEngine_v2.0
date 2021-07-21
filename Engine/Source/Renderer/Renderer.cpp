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

    m_pContext = GraphicsContext::createContext(configs.api);
    
    if (!m_pContext) {

        R_ERR("Renderer", "Failed to create graphics context, aborting...");

        return;
    }

    info.engineName = "Recluse Engine";
    info.appName    = "NoName";

    result = m_pContext->initialize(info, flags);

    if (result != REC_RESULT_OK) {

        R_ERR("Renderer", "Failed to initialize context!");        

    }

    std::vector<GraphicsAdapter*> adapters = m_pContext->getGraphicsAdapters();
    
    determineAdapter(adapters);
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
    }
}
} // Engine
} // Recluse