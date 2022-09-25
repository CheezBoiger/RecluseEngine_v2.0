//
#include "Recluse/Game/Components/Transform.hpp"

#include "Recluse/Game/Systems/TransformSystem.hpp"

namespace Recluse {

R_COMPONENT_IMPLEMENT(Transform, TransformSystem);


void Transform::onCleanUp()
{
    Transform::free(this);
}

} // Recluse