//
#include "Recluse/Game/Systems/TransformSystem.hpp"

namespace Recluse {

ErrType TransformSystem::onAllocateComponent(Transform** pOut)
{
    // TODO: We are just testing it out. We would need to hold onto this handle!
    *pOut = new Transform();
    m_transforms.push_back(*pOut);
    return R_RESULT_OK;
}


ErrType TransformSystem::onFreeComponent(Transform** pIn)
{
    U32 i = 0;
    for (auto& it = m_transforms.begin(); it != m_transforms.end(); ++it)
    {
        if ((*pIn)->getOwner() == (*it)->getOwner())
        {
            m_transforms.erase(it);
            delete* pIn;
            *pIn = nullptr;
            return R_RESULT_OK;
        }
    }
    return R_RESULT_OK;
}


void TransformSystem::onUpdateComponents(const RealtimeTick& tick)
{
    for (auto& transform : m_transforms)
    {
        transform->updateMatrices();
    }
}
} // Recluse