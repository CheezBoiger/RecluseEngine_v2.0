//
#pragma once

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Game/Components/Camera.hpp"

namespace Recluse {
namespace Engine {


class CameraSystem : public ECS::System<Camera>
{
public:
    R_DECLARE_GAME_SYSTEM(CameraSystem);
    
    virtual ~CameraSystem() { onCleanUp(); }

    virtual ResultCode onCleanUp() override;
    virtual ResultCode onInitialize(MessageBus* bus) override;
    virtual void onUpdate(ECS::Registry* registry, const RealtimeTick& tick) override;

private:

};
} // Engine
} // Recluse