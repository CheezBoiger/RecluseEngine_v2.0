//
#include "Recluse/Scene/Camera.hpp"

namespace Recluse {
namespace Engine {


void Camera::updateView(const Transform* transform)
{
    if ((transform->position != m_position).all())
    {
        m_position = transform->position;
    }

    if ((transform->rotation != m_rotation).all())
    {
    }
}
} // Engine
} // Recluse