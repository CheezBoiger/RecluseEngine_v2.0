//
#pragma once

#include "Recluse/Math/Plane.hpp"
#include "Recluse/Math/Matrix44.hpp"

namespace Recluse {
namespace Math {

struct Ray;
struct Bounds3d;


// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
//
// Specifies the following frustum diagram:
//
// 
//                 +-----------------------+
//                /|                      /|
//               / |         f           / |
//              /  |           t        /  |
//             /   |                   /   |
//            +-----------------------+  r |
//        l   |    +------------------|----+
//            |   /                   |   /
//            |  /       n    b       |  /
//            | /                     | /
//            |/                      |/
//            +-----------------------+
struct Frustum 
{
    enum FrustumPlane
    {
        FACE_LEFT,
        FACE_RIGHT,
        FACE_TOP,
        FACE_BOTTOM,
        FACE_NEAR,
        FACE_FAR,
        FACE_PLANES_COUNT = 6
    };

    // 
    Plane faces[FACE_PLANES_COUNT];
};


// Check whether we intersect any of the faces of the frustum.
R_PUBLIC_API Bool intersects(const Frustum& frustum, const Bounds3d& bounds);
} // Math
} // Recluse