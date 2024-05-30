//
#include "Recluse/Game/Systems/CameraSystem.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/Components/Camera.hpp"

namespace Recluse {
namespace Engine {


void CameraSystem::onUpdate(Engine::Scene* scene, const RealtimeTick& tick)
{
    // To iterate for all components in the scene, obtain all components from the registry.
    ECS::Registry<Camera>* cameraRegistry = scene->getRegistry<Camera>();
    if (cameraRegistry)
    {
        std::vector<Camera*> cameras = cameraRegistry->getAllComponents();
        for (U32 i = 0; i < cameras.size(); ++i)
        {
            Camera* camera = cameras[i];
            const RGUID entity = camera->getOwner();
            std::tuple<Transform*> tuple = obtainTuple<Transform>(scene, entity);
            Transform* transform = std::get<Transform*>(tuple);
            if (transform)
            {
                camera->update(transform);
            }
        }
    }
}
} // Engine
} // Recluse