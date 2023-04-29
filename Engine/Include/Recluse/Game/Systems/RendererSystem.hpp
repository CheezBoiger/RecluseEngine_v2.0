//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Game/GameSystem.hpp"

#include "Recluse/Game/Components/RendererComponent.hpp"

namespace Recluse {


class RendererSystem : public ECS::System<RendererComponent>
{
public:

    virtual         ~RendererSystem() { }

    virtual ResultCode allocateComponent(RendererComponent** pOut) override;
    virtual ResultCode freeComponent(RendererComponent** pOut)     override;
    
    virtual ResultCode onInitialize()                              override;
    virtual ResultCode onCleanUp()                                 override;
    virtual void    updateComponents(F32 deltaTime)             override;
    virtual ResultCode clearAll()                                  override;
};
} // Recluse