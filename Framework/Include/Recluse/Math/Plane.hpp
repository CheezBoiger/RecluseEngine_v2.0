//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Math/Vector4.hpp"

namespace Recluse {


// A plane consists of the equation:
// Ax + By + Cz + d = 0
// 
struct Plane
{
    Float3 N;
    F32    d;

    // Obtain the signed distance from this plane.
    // Usually positive.
    // https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    F32 signedDistanceTo(const Float3& p) const
    {
        return dot(N, p) - d;
    }
};
} // Recluse