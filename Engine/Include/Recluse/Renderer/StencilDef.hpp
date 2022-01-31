//
#pragma once
#include "Recluse/Renderer/RenderCommon.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


enum StencilFlag
{
    STENCIL_FLAG_BACKGROUND = (1 << 0),
    STENCIL_FLAG_FOREGROUND = (1 << 1)
};

typedef U32 StencilFlags;
} // Recluse