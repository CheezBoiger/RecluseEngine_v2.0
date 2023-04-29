//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"

#include "GUIContext.hpp"

#define WXUSINGDLL 1
#define wxUSE_MENUBAR 1
#define wxUSE_MENUS 1
#define wxUSE_STATUSBAR 1

#include "wx/wxprec.h"
#include "wx/frame.h"
#include "wx/menu.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

namespace Recluse {


class WxGuiContext : public Editor::IGUIContext
{
public:
    virtual ResultCode onRun(int c, char* argv[]) override;
    virtual ResultCode onSetUp() override;
    virtual ResultCode onTearDown() override;
};
} // Recluse