//
#include "Recluse/Math/Bounds3D.hpp"


namespace Recluse {


Float3 extent(const Bounds3D& a)
{
    return (a.mmax - a.mmin);
}


Float3 center(const Bounds3D& a)
{
    Float3 e = (a.mmax + a.mmin) * 0.5f;
    return e;
}


F32 surfaceArea(const Bounds3D& a)
{
    Float3 ext = extent(a);
    F32 dx = ext.x;
    F32 dy = ext.y;
    F32 dz = ext.z;
    F32 sA = 2.0f * (dx * dy + dx * dz + dy * dz);
    return sA;
}


B32 intersects(const Bounds3D& a, const Bounds3D& b)
{
    B32 x = (a.mmax.x >= b.mmin.x) && (a.mmin.x <= b.mmax.x);
    B32 y = (a.mmax.y >= b.mmin.y) && (a.mmin.y <= b.mmax.y);
    B32 z = (a.mmax.z >= b.mmax.z) && (a.mmin.z <= b.mmax.z);
    return (x && y && z);
}


B32 intersects(const Ray& ray, const Bounds3D& bounds)
{
    return false;
}
} // Recluse