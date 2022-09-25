//
#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Math/Ray.hpp"
#include "Recluse/Math/Plane.hpp"

namespace Recluse {


Float3 extent(const Bounds3d& a)
{
    return (a.mmax - a.mmin);
}


Float3 center(const Bounds3d& a)
{
    Float3 e = (a.mmax + a.mmin) * 0.5f;
    return e;
}


F32 surfaceArea(const Bounds3d& a)
{
    Float3 ext  = extent(a);
    F32 dx      = ext.x;
    F32 dy      = ext.y;
    F32 dz      = ext.z;
    F32 sA      = 2.0f * (dx * dy + dx * dz + dy * dz);
    return sA;
}


Bool intersects(const Bounds3d& a, const Bounds3d& b)
{
    Bool x = (a.mmax.x >= b.mmin.x) && (a.mmin.x <= b.mmax.x);
    Bool y = (a.mmax.y >= b.mmin.y) && (a.mmin.y <= b.mmax.y);
    Bool z = (a.mmax.z >= b.mmax.z) && (a.mmin.z <= b.mmax.z);
    return (x && y && z);
}


Bool intersects(const Ray& ray, const Bounds3d& bounds, F32& t)
{
	Float3 dirInv   = 1.0f / ray.dir;

	F32 t1          = (bounds.mmin.x - ray.o.x) * dirInv.x;
	F32 t2          = (bounds.mmax.x - ray.o.x) * dirInv.x;
	F32 t3          = (bounds.mmin.y - ray.o.y) * dirInv.y;
	F32 t4          = (bounds.mmax.y - ray.o.y) * dirInv.y;
	F32 t5          = (bounds.mmin.z - ray.o.z) * dirInv.z;
	F32 t6          = (bounds.mmax.z - ray.o.z) * dirInv.z;

	F32 tmin        = maximum(maximum(minimum(t1, t2), minimum(t3, t4)), minimum(t5, t6));
	F32 tmax        = minimum(minimum(maximum(t1, t2), minimum(t3, t4)), maximum(t5, t6));

	if (tmax < 0.f) { t = tmax; return false; }
	if (tmin > tmax){ t = tmax; return false; }

	t = tmin;
	return true;
}


Bool intersects(const Plane& plane, const Bounds3d& bounds)
{
    // Plane - AABB intersection algorithm
    // To understand how this works, link below.
    // Credits: https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
    Float3 c    = center(bounds);
    Float3 e    = bounds.mmax - c;
    Float3 n    = plane.N;

    F32 r       = e.x * fabs(plane.N[0]) + e.y * fabs(plane.N[1]) + e.z * fabs(plane.N[2]);
    F32 s       = dot(n, c);
    return (fabs(s) <= r);
}
} // Recluse