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

    virtual ErrType allocateComponent(RendererComponent** pOut) override;
    virtual ErrType freeComponent(RendererComponent** pOut)     override;
    
    virtual ErrType onInitialize()                              override;
    virtual ErrType onCleanUp()                                 override;
    virtual void    updateComponents(F32 deltaTime)             override;
    virtual ErrType clearAll()                                  override;
};
} // Recluse