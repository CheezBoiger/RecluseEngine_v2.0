//
#pragma once

#include "Recluse/Math/Plane.hpp"
#include "Recluse/Math/Matrix44.hpp"

#include "RecluseFramework_exports.hpp"

namespace Recluse {
namespace Math {

struct Ray3d;
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
    enum Face
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

    const Plane& operator[](Face face) const 
    {
        return faces[face];
    }
};


// Check whether we intersect any of the faces of the frustum.
RecluseFramework_PUBLIC_API Bool intersects(const Frustum& frustum, const Bounds3d& bounds);
} // Math
} // Recluse