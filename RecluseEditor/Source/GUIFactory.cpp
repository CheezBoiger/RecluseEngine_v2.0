//
#include "GUIFactory.hpp"

#include "WxWidgets/WxContext.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace GUIContext {

IGUIDriver* gDriver = nullptr;
IGUIContext* gGlobalContext = nullptr;


IGUIContext* get()
{
    return gGlobalContext;
}


IGUIDriver* getDriver()
{
    return gDriver;
}
} // GUIContext


namespace GUIFactory {

ErrType createContext(GUIContext ctx)
{
    using namespace Recluse::GUIContext;

    if (!gGlobalContext && !gDriver)
    {
        switch (ctx)
        {
        case Context_WxWidgets:
            gGlobalContext = new WxGuiContext();
            gDriver = nullptr;
            return R_RESULT_OK;
        case Context_Null:
        default:
            return R_RESULT_FAILED;
        }
    }


    R_ERR("Editor", "Contact and/or driver are already initialized!! Ignoring this call, passing null...");
    return R_RESULT_FAILED;
}


void destroyContext(IGUIContext* pCtx)
{
    using namespace Recluse::GUIContext;

    if (pCtx) delete pCtx;
    if (gDriver) delete gDriver;

    gGlobalContext  = nullptr;
    gDriver         = nullptr;
}
} // GUIFactory
} // Recluse