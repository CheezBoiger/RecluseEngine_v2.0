//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Game/GameSystem.hpp"

#include "Recluse/Game/Components/RendererComponent.hpp"

namespace Recluse {


class R_PUBLIC_API RendererSystem : public ECS::System
{
public:
    R_DECLARE_GAME_SYSTEM(RendererSystem);

    virtual         ~RendererSystem() { }

    virtual ResultCode  onInitialize(MessageBus* bus) override { return RecluseResult_NoImpl; }
    virtual ResultCode  onCleanUp()                   override;
    virtual void        onUpdate(const RealtimeTick& tick)  override;
};
} // Recluse