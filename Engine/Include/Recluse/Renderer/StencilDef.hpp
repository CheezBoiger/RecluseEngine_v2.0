//
#pragma once
#include "Recluse/Renderer/RenderCommon.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


// Stencil identifiers.
enum Stencil
{
    Stencil_Background  = (1 << 0),
    Stencil_Foreground  = (1 << 1),
    Stencil_Reserved0   = (1 << 2),
    Stencil_Reserved1   = (1 << 3),
    Stencil_Reserved2   = (1 << 4),
    Stencil_Reserved3   = (1 << 5),
    Stencil_Reserved4   = (1 << 6),
    Stencil_Reserved5   = (1 << 7),

    Stencil_Mask        = (0xFF)

    // Extended will be for 32 bit render targets.
};

typedef U32 StencilFlags;
} // Recluse