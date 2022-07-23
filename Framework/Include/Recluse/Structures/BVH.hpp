//
#pragma once

#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Math/Ray.hpp"
#include "Recluse/Types.hpp"

#include <vector>

namespace Recluse {


template<typename Type>
class BoundingVolumeHierarchy
{
	struct Node
	{
		Type data;
		struct Node* pLeft;
		struct Node* pRight;
	};
public:


	Bool intersects(const Ray& ray);

	void build();



	void reBuild();


private:
	U32 m_totalNodes;
	std::vector<Node> m_nodes;
};
} // Recluse