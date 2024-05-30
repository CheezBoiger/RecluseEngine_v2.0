//
#include "Recluse/Game/Components/Camera.hpp"

namespace Recluse {
namespace Engine {


void Camera::update(const Transform* transform)
{
    if (m_updateFlags & CameraUpdate_View)
    {
        Math::Float3 position = transform->position;
        Math::Float3 right = transform->right;
        Math::Float3 up = transform->up;
        Math::Float3 forward = transform->forward;

        // Construct the camera view matrix.
        m_View = Math::Matrix44(
            right.x,                        up.x,                       forward.x,                      0.0f,
            right.y,                        up.y,                       forward.y,                      0.0f,
            right.z,                        up.z,                       forward.z,                      0.0f,
            -Math::dot(right, position),    -Math::dot(up, position),   -Math::dot(forward, position),  1.0f
        );
    }
    
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
        m_InverseProjection = Math::inverse(m_Projection);
    }

    if (m_updateFlags & (CameraUpdate_Projection | CameraUpdate_View))
    {
        m_ViewProjection = m_View * m_Projection;   
        m_InverseViewProjection = Math::inverse(m_ViewProjection);
    }
}


ResultCode CameraRegistry::onAllocateComponent(const RGUID& owner) 
{
    Camera* cam = getComponent(owner);
    if (cam)
    {
        return RecluseResult_AlreadyExists;
    }
    cam = new Camera();
    activeCameras.push_back(cam);
    cameraMap.insert(std::make_pair(owner, activeCameras.size() - 1));
    return RecluseResult_Ok;
}


ResultCode CameraRegistry::onFreeComponent(const RGUID& owner)
{
    auto it = cameraMap.find(owner);
    if (it == cameraMap.end())
    {
        return RecluseResult_NotFound;
    }
    // TODO: Need to be wiser about deleting cameras from the vector. Might require having free slots available.
    delete activeCameras[it->second];
    cameraMap.erase(it);
    return RecluseResult_Ok;
}


Camera* CameraRegistry::getComponent(const RGUID& entityKey)
{
    auto it = cameraMap.find(entityKey);
    if (it == cameraMap.end())
    {
        return nullptr;
    }
    return activeCameras[it->second];
}
} // Engine
} // Recluse