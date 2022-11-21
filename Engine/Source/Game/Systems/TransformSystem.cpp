//
#include "Recluse/Game/Systems/TransformSystem.hpp"

namespace Recluse {

ErrType TransformSystem::onAllocateComponent(Transform** pOut)
{
    // TODO: We are just testing it out. We would need to hold onto this handle!
    *pOut = new Transform();
    m_transforms.push_back(*pOut);
    return RecluseResult_Ok;
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
            return RecluseResult_Ok;
        }
    }
    return RecluseResult_Ok;
}


void TransformSystem::onUpdateComponents(const RealtimeTick& tick)
{
    for (auto& transform : m_transforms)
    {
        transform->updateMatrices();
    }
}


Transform* TransformSystem::getComponent(const RGUID& entityKey)
{
    return nullptr;
}


Transform** TransformSystem::getAllComponents(U64& pOut)
{
    return m_transforms.data();
}
} // Recluse