//
#pragma once

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Game/Components/Transform.hpp"

namespace Recluse {


class TransformSystem : public ECS::SystemDefinition<Transform>
{
public:
    virtual         ~TransformSystem() { }

    virtual ErrType onInitialize()                      override;
    virtual ErrType onCleanUp()                         override;
    virtual void    updateComponents(F32 deltaTime)     override;
    virtual ErrType allocateComponent(Transform** pOut) override;
    virtual ErrType freeComponent(Transform** pIn)      override;
};
} // Recluse