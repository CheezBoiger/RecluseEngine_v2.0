//
#pragma once


#include "Recluse/Types.hpp"
#include "GUIContext.hpp"


namespace Recluse {
namespace GUIFactory {

enum GUIContext
{
    Context_WxWidgets,
    Context_Null
};

R_PUBLIC_API ErrType         createContext(GUIContext ctx);
R_PUBLIC_API void            destroyContext(IGUIContext* pCtx);
} // GUIFactory
} // Recluse