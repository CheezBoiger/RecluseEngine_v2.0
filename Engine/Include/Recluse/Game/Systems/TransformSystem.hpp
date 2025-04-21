//
#pragma once

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Game/Components/Transform.hpp"

#include <vector>

namespace Recluse {
namespace Engine {
// Transform system handles updating all transform components in the 
// world.
class RecluseEngine_PUBLIC_API TransformSystem : public ECS::System<Transform>
{
public:
    R_DECLARE_GAME_SYSTEM(TransformSystem);

    virtual             ~TransformSystem() { onCleanUp(); }

    virtual ResultCode     onInitialize()                                                                       override;
    virtual ResultCode     onCleanUp()                                                                          override;
    virtual void           onUpdate(ECS::Registry* registry, const RealtimeTick& tick, Engine::Scene* scene)    override;
    virtual ResultCode     onEvent(const EventMessage& event) override;

private:
    Bool m_doUpdate = false;
};
} // Engine
} // Recluse