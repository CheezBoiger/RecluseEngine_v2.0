//
#include "Recluse/Game/Components/Transform.hpp"

#include "Recluse/Game/Systems/TransformSystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"

namespace Recluse {

using namespace Math;

void Transform::onCleanUp()
{
    Super::onCleanUp();
}

ResultCode Transform::serialize(Archive* pArchive) const
{
    pArchive->write(&position,      sizeof(Float3));
    pArchive->write(&localPosition, sizeof(Float3));
    pArchive->write(&rotation,      sizeof(Quaternion));
    pArchive->write(&localRotation, sizeof(Quaternion));
    pArchive->write(&eulerAngles,   sizeof(Float3));
    pArchive->write(&forward,       sizeof(Float3));
    pArchive->write(&right,         sizeof(Float3));
    pArchive->write(&up,            sizeof(Float3));
    return RecluseResult_NoImpl;
}

ResultCode Transform::deserialize(Archive* pArchive)
{
    pArchive->read(&position,       sizeof(Float3));
    pArchive->read(&localPosition,  sizeof(Float3));
    pArchive->read(&rotation,       sizeof(Quaternion));
    pArchive->read(&localRotation,  sizeof(Quaternion));
    pArchive->read(&eulerAngles,    sizeof(Quaternion));
    pArchive->read(&forward,        sizeof(Float3));
    pArchive->read(&right,          sizeof(Float3));
    pArchive->read(&up,             sizeof(Float3));
    return RecluseResult_NoImpl;
}

void Transform::updateMatrices()
{
    Matrix44 t      = Math::translate(Matrix44::identity(), position);
    Matrix44 s      = Math::scale(t, scale);
    Matrix44 r      = Math::quatToMat44(rotation);
    Matrix44 World  = s * r * t;

    m_localToWorld  = World;
    m_worldToLocal  = Math::inverse(World);
}


ResultCode TransformRegistry::onCleanUp()
{
    for (auto it = m_transforms.begin(); it != m_transforms.end(); ++it)
    {
        (*it)->cleanUp();
        delete *it;
    }
    m_transforms.clear();
    return RecluseResult_Ok;
}


Transform* TransformRegistry::getComponent(const RGUID& entityKey)
{
    for (auto it = m_transforms.begin(); it != m_transforms.end(); ++it)
    {
        if ((*it)->getOwner() == entityKey)
            return *it;
    }
    return nullptr;
}


std::vector<Transform*> TransformRegistry::getAllComponents()
{
    return std::vector<Transform*>(m_transforms);
}


ResultCode TransformRegistry::onAllocateComponent(const RGUID& owner)
{
    // TODO: We are just testing it out. We would need to hold onto this handle!
    Transform* pTransform = new Transform();
    pTransform->setOwner(owner);
    m_transforms.push_back(pTransform);
    return RecluseResult_Ok;
}


ResultCode TransformRegistry::onFreeComponent(const RGUID& owner)
{
    U32 i = 0;
    for (auto& it = m_transforms.begin(); it != m_transforms.end(); ++it)
    {
        if (owner == (*it)->getOwner())
        {
            delete *it;
            m_transforms.erase(it);
            return RecluseResult_Ok;
        }
    }
    return RecluseResult_Ok;
}
} // Recluse