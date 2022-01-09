//
#pragma once

#include "Recluse/Game/Component.hpp"

#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Quaternion.hpp"

namespace Recluse {


class Transform : public Component 
{
public:

    // The World position of the transform.
    Float3 position;

    // The local position, relative to the parent.
    Float3 localPosition;

    // 
    Quaternion rotation;

    //
    Quaternion localRotation;

    // 
    Float3 eulerAngles;

private:
    // Local to World Matrix.
    Matrix44 m_localToWorld;

    // World to Local Matrix.
    Matrix44 m_worldToLocal;
};
} // Recluse