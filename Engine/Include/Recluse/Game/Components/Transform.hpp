//
#pragma once

#include "Recluse/Game/Component.hpp"

#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Matrix43.hpp"
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Quaternion.hpp"
#include "Recluse/RGUID.hpp"

#include <vector>
#include <unordered_map>

namespace Recluse {

using namespace Math;

// Transform component that stores transformations
// and positioning of a given entity.
class Transform : public ECS::Component
{
public:
    R_COMPONENT_DECLARE(Transform);

    virtual ~Transform() { }
    Transform() { }

    REDITOR(RATTRIBUTE("visible", "public"),
             RATTRIBUTE("default", Float3(0.f, 0.f, 0.f)),
             RATTRIBUTE("description", "World position of the transform"))
    Float3      position;       // The World position of the transform.
    REDITOR(RATTRIBUTE("visible", "public"))
    Float3      localPosition;  // The local position, relative to the parent.
    REDITOR(RATTRIBUTE("visible", "public"))
    Quaternion  rotation;       // Rotation of the transform in world.
    REDITOR(RATTRIBUTE("visible", "public"))
    Quaternion  localRotation;  // local rotation relative to the parent.
    REDITOR(RATTRIBUTE("visible", "public"))
    Float3      eulerAngles;    // rotation represented in euler angles (roll, pitch, yaw.)
    REDITOR(RATTRIBUTE("visible", "public"))
    Float3      forward;        // forward facing vector of the object.
    REDITOR(RATTRIBUTE("visible", "public"))
    Float3      right;          // right facing vector of the object.
    REDITOR(RATTRIBUTE("visible", "public"))
    Float3      up;             // up facing vector of the object.
    Float3      scale;          // scale vector of the object.

    virtual void                onCleanUp() override;

    Matrix44                    getLocalToWorld() const { return static_cast<Matrix44>(m_localToWorld); }
    Matrix44                    getWorldToLocal() const { return m_worldToLocal; }

    virtual ResultCode          serialize(Archive* pArchive) const override;
    virtual ResultCode          deserialize(Archive* pArchive) override;

    void                        updateMatrices(const Transform* parentTransform = nullptr);

private:
    Matrix43                    m_localToWorld;        // Local to World Matrix.
    Matrix43                    m_prevLocalToWorld;    // Previous Local To World.

    Matrix44                    m_worldToLocal;        // World to Local Matrix.
};


class R_PUBLIC_API TransformRegistry : public ECS::Registry<Transform>
{
public:
    R_COMPONENT_REGISTRY_DECLARE(TransformRegistry);

    virtual ResultCode              onAllocateComponent(const RGUID& owner)     override;
    virtual ResultCode              onFreeComponent(const RGUID& owner)         override;

    virtual Transform*              getComponent(const RGUID& entityKey)        override;
    virtual std::vector<Transform*> getAllComponents()                          override;

    virtual ResultCode              onInitialize()                              override { return RecluseResult_NoImpl; }
    virtual ResultCode              onCleanUp()                                 override;

private:
    //std::unordered_map<RGUID, Transform*> m_table;
    std::vector<Transform*> m_transforms;
};
} // Recluse