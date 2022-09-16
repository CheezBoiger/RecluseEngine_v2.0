//
#include "Recluse/Math/Frustum.hpp"
#include "Recluse/Math/Bounds3D.hpp"

namespace Recluse {


Bool intersects(const Frustum& frustum, const Bounds3d& bounds)
{
    // Check all faces of the frustum, to see if we are inside.
    // This must be done for all 6 sides.
    return intersects(frustum.faces[Frustum::FACE_LEFT], bounds) &&
           intersects(frustum.faces[Frustum::FACE_RIGHT], bounds) &&
           intersects(frustum.faces[Frustum::FACE_TOP], bounds) &&
           intersects(frustum.faces[Frustum::FACE_BOTTOM], bounds) &&
           intersects(frustum.faces[Frustum::FACE_NEAR], bounds) &&
           intersects(frustum.faces[Frustum::FACE_FAR], bounds);
}
} // Recluse