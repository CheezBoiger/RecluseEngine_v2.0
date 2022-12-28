//
#include "GUIFactory.hpp"

#include "WxWidgets/WxContext.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Editor {
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
    using namespace Recluse::Editor::GUIContext;

    if (!gGlobalContext && !gDriver)
    {
        switch (ctx)
        {
        case Context_WxWidgets:
            gGlobalContext = new WxGuiContext();
            gDriver = nullptr;
            return RecluseResult_Ok;
        case Context_Null:
        default:
            return RecluseResult_Failed;
        }
    }


    R_ERR("Editor", "Contact and/or driver are already initialized!! Ignoring this call, passing null...");
    return RecluseResult_Failed;
}


void destroyContext(IGUIContext* pCtx)
{
    using namespace Recluse::Editor::GUIContext;

    if (pCtx) delete pCtx;
    if (gDriver) delete gDriver;

    gGlobalContext  = nullptr;
    gDriver         = nullptr;
}
} // Editor
} // GUIFactory
} // Recluse