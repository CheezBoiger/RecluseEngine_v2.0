//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Bounds3D.hpp"

namespace Recluse {


class Octree
{
public:

    void insert(const Float3& point);
    void remove(const Float3& point);

    Bool find(const Float3& point);

private:
    struct OctreeNode
    {
        struct OctreeNode*  nodes[8];
        Bounds3d            region;
    };
};
} // Recluse