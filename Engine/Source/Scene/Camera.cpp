//
#include "Recluse/Scene/Camera.hpp"

namespace Recluse {
namespace Engine {


void Camera::updateView(const Transform* transform)
{
    if ((transform->position != m_position).all())
    {
    }
}
} // Engine
} // Recluse