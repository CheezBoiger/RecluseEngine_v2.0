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

void Transform::updateMatrices(const Transform* parentTransform)
{
    Matrix44 World = Matrix44::identity();
    Matrix44 s = Math::scale(Matrix44::identity(), scale);

    if (parentTransform)
    {
        Matrix44 t = Math::translate(Matrix44::identity(), localPosition);
        Matrix44 r = Math::quatToMat44(localRotation);
        World = s * r * t;
    }
    else
    {
        Matrix44 t      = Math::translate(Matrix44::identity(), position);
        Matrix44 r      = Math::quatToMat44(rotation);
        World  = s * r * t;
    }

    m_localToWorld  = World;
    m_worldToLocal  = Math::inverse(World);
}


ResultCode TransformRegistry::onCleanUp()
{
    for (auto it : m_table)
    {
        it.second.cleanUp();
    }
    m_table.clear();
    return RecluseResult_Ok;
}


Transform* TransformRegistry::getComponent(const RGUID& entityKey)
{
    auto it = m_table.find(entityKey);
    if (it != m_table.end())
        return &it->second;
    return nullptr;
}


std::vector<Transform*> TransformRegistry::getAllComponents()
{
    std::vector<Transform*> transforms(m_table.size());
    U32 i = 0;
    for (std::map<RGUID, Transform, RGUID::Less>::iterator it = m_table.begin(); it != m_table.end(); ++it)
    {
        transforms[i++] = &it->second;
    }
    return transforms;
}


ResultCode TransformRegistry::onAllocateComponent(const RGUID& owner)
{
    // TODO: We are just testing it out. We would need to hold onto this handle!
    auto it = m_table.find(owner);
    if (it == m_table.end())
    {
        m_table[owner] = Transform();
        m_table[owner].setOwner(owner);
        return RecluseResult_Ok;
    }
    return RecluseResult_AlreadyExists;
}


ResultCode TransformRegistry::onFreeComponent(const RGUID& owner)
{
    auto it = m_table.find(owner);
    if (it != m_table.end())
    {
        it->second.cleanUp();
        m_table.erase(it);
    }
    return RecluseResult_Ok;
}
} // Recluse