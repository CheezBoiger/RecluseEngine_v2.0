//
#pragma once

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Game/Components/Transform.hpp"

namespace Recluse {

class R_PUBLIC_API TransformSystem : public ECS::System<Transform>
{
public:
    R_DECLARE_GAME_SYSTEM(TransformSystem(), Transform);

    virtual         ~TransformSystem() { }

    virtual ErrType onInitialize()                          override { return R_RESULT_NO_IMPL; }
    virtual ErrType onCleanUp()                             override { return R_RESULT_NO_IMPL; }
    virtual void    onUpdateComponents(F32 deltaTime)         override { }
    virtual ErrType onAllocateComponent(Transform** pOut)   override;
    virtual ErrType onAllocateComponents(Transform*** pOuts, U32 count) override { return R_RESULT_NO_IMPL; }
    virtual ErrType onFreeComponent(Transform** pIn)        override;
    virtual ErrType onFreeComponents(Transform*** pOuts, U32 count) override { return R_RESULT_NO_IMPL; }
};
} // Recluse