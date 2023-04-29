//
#pragma once

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Game/Components/Transform.hpp"

#include <vector>

namespace Recluse {

// Transform system handles updating all transform components in the 
// world.
class R_PUBLIC_API TransformSystem : public ECS::System<Transform>
{
public:
    R_DECLARE_GAME_SYSTEM(TransformSystem(), Transform);

    virtual             ~TransformSystem() { onCleanUp(); }

    virtual ResultCode     onInitialize()                          override { return RecluseResult_NoImpl; }
    virtual ResultCode     onCleanUp()                             override { return RecluseResult_NoImpl; }
    virtual void        onUpdateComponents(const RealtimeTick& tick)         override;
    virtual ResultCode     onAllocateComponent(Transform** pOut)   override;
    virtual ResultCode     onAllocateComponents(Transform*** pOuts, U32 count) override { return RecluseResult_NoImpl; }
    virtual ResultCode     onFreeComponent(Transform** pIn)        override;
    virtual ResultCode     onFreeComponents(Transform*** pOuts, U32 count) override { return RecluseResult_NoImpl; }

    virtual Transform*  getComponent(const RGUID& entityKey) override;
    virtual Transform** getAllComponents(U64& pOut) override;

private:
    std::vector<Transform*> m_transforms;
};
} // Recluse