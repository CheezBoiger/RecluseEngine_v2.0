//
#pragma once

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Game/Components/Transform.hpp"

#include <vector>

namespace Recluse {

// Transform system handles updating all transform components in the 
// world.
class R_PUBLIC_API TransformSystem : public ECS::System<Transform>
{
public:
    R_DECLARE_GAME_SYSTEM(TransformSystem);

    virtual             ~TransformSystem() { onCleanUp(); }

    virtual ResultCode     onInitialize(MessageBus* bus)                        override;
    virtual ResultCode     onCleanUp()                                          override;
    virtual void           onUpdate(ECS::Registry* registry, const RealtimeTick& tick)                   override;

private:
    Bool m_doUpdate = false;
};
} // Recluse