//
#pragma once

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Game/Components/Transform.hpp"

#include <vector>

namespace Recluse {

// Transform system handles updating all transform components in the 
// world.
class R_PUBLIC_API TransformSystem : public ECS::System
{
public:
    R_DECLARE_GAME_SYSTEM(TransformSystem);

    virtual             ~TransformSystem() { onCleanUp(); }

    virtual ResultCode     onInitialize(MessageBus* bus)                        override;
    virtual ResultCode     onCleanUp()                                          override;
    virtual void           onUpdate(Engine::Scene* scene, const RealtimeTick& tick)                   override;

private:
    Bool m_doUpdate = false;
};
} // Recluse