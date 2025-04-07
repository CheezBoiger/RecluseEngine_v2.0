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
    virtual ResultCode onInitialize() override;
    virtual void onUpdate(ECS::Registry* registry, const RealtimeTick& tick, Scene* scene) override;

private:

};
} // Engine
} // Recluse