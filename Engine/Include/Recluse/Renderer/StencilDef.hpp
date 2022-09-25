//
#pragma once
#include "Recluse/Renderer/RenderCommon.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


enum StencilFlag
{
    StencilFlag_Background = (1 << 0),
    StencilFlag_Foreground = (1 << 1)
};

typedef U32 StencilFlags;
} // Recluse