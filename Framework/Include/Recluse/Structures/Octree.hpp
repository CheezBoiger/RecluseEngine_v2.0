//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Bounds3D.hpp"

namespace Recluse {


class Octree
{
public:

    void insert(const Math::Float3& point);
    void remove(const Math::Float3& point);

    Bool find(const Math::Float3& point);

private:
    struct OctreeNode
    {
        struct OctreeNode*  nodes[8];
        Math::Bounds3d      region;
    };
};
} // Recluse