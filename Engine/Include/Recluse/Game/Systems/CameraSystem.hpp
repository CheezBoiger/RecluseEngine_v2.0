//
#pragma once

#include "Recluse/Game/GameSystem.hpp"


namespace Recluse {
namespace Engine {


class CameraSystem : public ECS::System
{
public:
    R_DECLARE_GAME_SYSTEM(CameraSystem);
    
    virtual ~CameraSystem() { onCleanUp(); }

    virtual ResultCode onCleanUp() override;
    virtual ResultCode onInitialize(MessageBus* bus) override;
    virtual void onUpdate(Engine::Scene* scene, const RealtimeTick& tick) override;

private:

};
} // Engine
} // Recluse