//
#include "Recluse/Game/Components/Transform.hpp"

#include "Recluse/Game/Systems/TransformSystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"

namespace Recluse {

R_COMPONENT_IMPLEMENT(Transform, TransformSystem);


void Transform::onCleanUp()
{
}

void Transform::onRelease()
{
    Transform::free(this);
}

void Transform::onUpdate(const RealtimeTick& tick) 
{
    Matrix44 t = Recluse::translate(Matrix44::identity(), position);
    Matrix44 s = Recluse::scale(t, scale);
    Matrix44 r = Recluse::quatToMat44(rotation);

    Matrix44 World = s * r * t;

    m_localToWorld = World;
    m_worldToLocal = Recluse::inverse(World);
}

ErrType Transform::serialize(Archive* pArchive)
{
    pArchive->write(&position, sizeof(Float3));
    pArchive->write(&localPosition, sizeof(Float3));
    pArchive->write(&rotation, sizeof(Quaternion));
    pArchive->write(&localRotation, sizeof(Quaternion));
    pArchive->write(&eulerAngles, sizeof(Float3));
    pArchive->write(&forward, sizeof(Float3));
    pArchive->write(&right, sizeof(Float3));
    pArchive->write(&up, sizeof(Float3));
    return R_RESULT_NO_IMPL;
}

ErrType Transform::deserialize(Archive* pArchive)
{
    pArchive->read(&position, sizeof(Float3));
    pArchive->read(&localPosition, sizeof(Float3));
    pArchive->read(&rotation, sizeof(Quaternion));
    pArchive->read(&localRotation, sizeof(Quaternion));
    pArchive->read(&eulerAngles, sizeof(Quaternion));
    pArchive->read(&forward, sizeof(Float3));
    pArchive->read(&right, sizeof(Float3));
    pArchive->read(&up, sizeof(Float3));
    return R_RESULT_NO_IMPL;
}
} // Recluse