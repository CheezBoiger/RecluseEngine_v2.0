//
#pragma once


#include "Recluse/Types.hpp"
#include "GUIContext.hpp"


namespace Recluse {
namespace Editor {
namespace GUIFactory {

enum GUIContext
{
    Context_WxWidgets,
    Context_Null
};

// Create a GUI context.
R_PUBLIC_API ErrType         createContext(GUIContext ctx);

// Destroy the GUI context.
R_PUBLIC_API void            destroyContext(IGUIContext* pCtx);
} // Editor
} // GUIFactory
} // Recluse