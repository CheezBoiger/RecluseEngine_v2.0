//
#pragma once

#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Math/Ray.hpp"
#include "Recluse/Types.hpp"

#include <vector>

namespace Recluse {

enum SplitMethod 
{
	SplitMethod_SAH,		// Surface Area Heuristic
	SplitMethod_HLBVH,		// Linear Bounding Volume Hierarchy, this is a parallel bvh build.
	SplitMethod_Middle,		// Middle point split method. Splits bounds down the middle.
	SplitMethod_EqualCounts	// Splits bounds into equal-sized subsets.
};


template<typename Type>
class BoundingVolumeHierarchy
{
	// Node contains the bounds of a given cluster or object.
	struct Node
	{
		// Offset to data structure containing the objects.
		U32				offset;
		// The number of objects in the list.
		U32				count;
		Math::Bounds3d	bounds;
		struct Node*	left;
		struct Node*	right;

		// Determine if is leaf. A leaf node is a node that contains no left or right child.
		bool			isLeaf() const { return !left && !right; }
	};
public:

	Bool				intersects(const Math::Ray3d& ray);
	void				build(SplitMethod splitMethod);

	void				rebuild();

private:
	U32					m_maxPrimitivesInNode;
	U32					m_totalNodes;
	SplitMethod			m_splitMethod;
	std::vector<Node>	m_nodes;

	// Objects in the BVH
	std::vector<Type>	m_primitives;
};
} // Recluse