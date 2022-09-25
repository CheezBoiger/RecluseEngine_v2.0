//
#include "Recluse/Game/Systems/TransformSystem.hpp"

namespace Recluse {

ErrType TransformSystem::onAllocateComponent(Transform** pOut)
{
    // TODO: We are just testing it out. We would need to hold onto this handle!
    *pOut = new Transform();
    return R_RESULT_OK;
}


ErrType TransformSystem::onFreeComponent(Transform** pIn)
{
    delete *pIn;
    *pIn = nullptr;
    return R_RESULT_OK;
}
} // Recluse