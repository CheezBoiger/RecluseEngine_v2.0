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

    virtual ResultCode     onInitialize()                                       override { return RecluseResult_NoImpl; }
    virtual ResultCode     onCleanUp()                                          override;
    virtual void           onUpdate(const RealtimeTick& tick)                   override;
};
} // Recluse