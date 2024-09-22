//
#pragma once


#include "Recluse/Types.hpp"
#include "GUIContext.hpp"

#include "RecluseEditorBase_exports.hpp"


namespace Recluse {
namespace Editor {
namespace GUIFactory {

enum GUIContext
{
    Context_WxWidgets,
    Context_Null
};

// Create a GUI context.
RecluseEditorBase_PUBLIC_API ResultCode      createContext(GUIContext ctx);

// Destroy the GUI context.
RecluseEditorBase_PUBLIC_API void            destroyContext(IGUIContext* pCtx);
} // Editor
} // GUIFactory
} // Recluse