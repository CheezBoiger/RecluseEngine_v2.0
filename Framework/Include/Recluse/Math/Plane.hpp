//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Math/Vector4.hpp"

namespace Recluse {
namespace Math {

// A plane consists of the equation:
// Ax + By + Cz + d = 0
// Where:
//          N = <A, B, C>
//          d = distance from origin.
struct Plane
{
    Float3 N;
    F32    d;

    Plane(const Float3& N = Float3(), F32 d = 0.f)
        : N(N)
        , d(d) { }

    // Obtain the signed distance from this plane.
    // Usually positive.
    // This equation is defined as:
    //      Distance = | A * x1 + B * y1 + C * z1 + d |
    //
    // Where: Distance          = distance from point to plane.
    //        N(A, B, C)        = Plane Normal.
    //        p(x1, y1, z1)     = Point p.
    //        d                 = -(A * x0 + B * y0 + C * z0)
    //        as (x0, y0, z0)   = some other point 
    // In this sense we can write | (N . p) - d | as it is the same equation.
    // Refer to https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_plane
    // for more info about this equation.
    //
    F32 signedDistanceTo(const Float3& p) const
    {
        return dot(N, p) - d;
    }
};


static Plane normalize(const Plane& plane)
{
    return Plane(normalize(plane.N), plane.d);
}
} // Math
} // Recluse