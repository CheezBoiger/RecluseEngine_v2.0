//
#pragma once

#include "Recluse/Game/Component.hpp"

#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Quaternion.hpp"


namespace Recluse {


// Transform component that stores transformations
// and positioning of a given entity.
class Transform : public ECS::Component 
{
public:
    virtual ~Transform() { }

    R_EDITOR_DECLARE("visible", "public", true)
    R_EDITOR_DECLARE("visible", "default", Float3(0.f, 0.f, 0.f))
    Float3      position;       // The World position of the transform.
    R_EDITOR_DECLARE("visible", "public", true)
    Float3      localPosition;  // The local position, relative to the parent.
    R_EDITOR_DECLARE("visible", "public", true)
    Quaternion  rotation;       // Rotation of the transform in world.
    R_EDITOR_DECLARE("visible", "public", true)
    Quaternion  localRotation;  // local rotation relative to the parent.
    R_EDITOR_DECLARE("visible", "public", true)
    Float3      eulerAngles;    // rotation represented in euler angles (roll, pitch, yaw.)

    // Update the transform.
    virtual void onUpdate(const RealtimeTick& tick) override;

private:
    Matrix44 m_localToWorld;        // Local to World Matrix.
    Matrix44 m_worldToLocal;        // World to Local Matrix.
    Matrix44 m_prevLocalToWorld;    // Previous Local To World.
    Matrix44 m_prevWorldToLocal;    // Previous World To Local.
};
} // Recluse