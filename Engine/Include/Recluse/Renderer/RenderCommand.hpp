//
#pragma once

#include "Recluse/Types.hpp"


namespace Recluse {
namespace Engine {


enum SurfaceType {
    SURFACE_TYPE_OPAQUE = (1 << 0),
    SURFACE_TYPE_TRANSPARENT = (1 << 1),
    SURFACE_TYPE_TRANSPARENT_CUTOUT = (1 << 2),
    SURFACE_TYPE_SUBSURFACE = (1 << 3),
    SURFACE_TYPE_FLAT = (1 << 4),
    SURFACE_TYPE_PARTICLE = (1 << 5)
};


enum ShadingType {
    SHADING_TYPE_PBR,
    SHADING_TYPE_FLAT
};


class RenderCommand {
public:
    
    
};
} // Engine
} // Recluse