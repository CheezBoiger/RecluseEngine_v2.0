//
#pragma once

#include "Recluse/Game/GameSystem.hpp"


namespace Recluse {
namespace Engine {


class LightSystem : public ECS::System
{
public:
    R_DECLARE_GAME_SYSTEM(LightSystem);

    void onUpdate(Engine::Scene* scene, const RealtimeTick& tick) override;
    ResultCode onCleanUp() override;
    void onClearAll() override;
    void onDrawDebug(Engine::Scene* scene, Engine::DebugRenderer* dr) override;
};
} // Engine
} // Recluse