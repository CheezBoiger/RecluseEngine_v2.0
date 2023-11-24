//
#include "Recluse/Scene/Camera.hpp"

namespace Recluse {
namespace Engine {


void Camera::update(const Transform* transform)
{
    if (m_updateFlags & CameraUpdate_Projection)
    {
        switch (m_projectionMode)
        {
            case CameraProjection_Orthographic:
                m_Projection = Math::orthographicLH(0, 0, 0, 0, 0, 0);
                break;
            case CameraProjection_Perspective:
            default:
                m_Projection = Math::perspectiveLH_Aspect(m_fov, m_aspect, m_near, m_far);
                break;
        }
        m_ViewProjection = m_View * m_Projection;
        m_InverseProjection = Math::inverse(m_Projection);
        m_InverseViewProjection = Math::inverse(m_ViewProjection);
    }
}
} // Engine
} // Recluse