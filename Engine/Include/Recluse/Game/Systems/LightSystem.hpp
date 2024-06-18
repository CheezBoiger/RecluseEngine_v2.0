//
#pragma once

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Game/Components/LightComponent.hpp"

namespace Recluse {
namespace Engine {


class LightSystem : public ECS::System<Light>
{
public:
    R_DECLARE_GAME_SYSTEM(LightSystem);

    void onUpdate(ECS::Registry* registry, const RealtimeTick& tick) override;
    ResultCode onCleanUp() override;
    void onClearAll() override;
    void onDrawDebug(ECS::Registry* registry, Engine::DebugRenderer* dr) override;
};
} // Engine
} // Recluse