//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Quaternion.hpp"
#include "Recluse/Math/Vector4.hpp"
#include <vector>

namespace Recluse {
namespace Engine {


struct Joint : public Serializable
{
    I32                 m_parent;
    I32                 m_globalNodeIndex;
    std::vector<I32>    m_children;
    std::string         m_name;
    Math::Matrix44      m_inverseBindMatrix;
    Math::Matrix44      m_undeformedNodeMatrix;
    
    Math::Float3        m_deformedNodeTranslation;
    Math::Quaternion    m_deformedNodeRotation;
    Math::Float3        m_deformedNodeScale;

    Math::Matrix44 getDeformedBindMatrix() const
    {
        Math::Matrix44 T = Math::translate(Math::Matrix44::identity(), m_deformedNodeTranslation);
        Math::Matrix44 R = Math::quatToMat44(m_deformedNodeRotation);
        Math::Matrix44 S = Math::scale(Math::Matrix44::identity(), m_deformedNodeScale);
        return m_undeformedNodeMatrix * (S * R * T);
    }
};
} // Engine
} // Recluse