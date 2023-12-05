//
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/Systems/TransformSystem.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Utility.hpp"

namespace Recluse {


R_DECLARE_GLOBAL_BOOLEAN(g_enableTransformLogging, false, "Transform.EnableLogging");

void TransformSystem::onUpdate(const RealtimeTick& tick)
{
    Engine::Scene* pScene = getScene();
    if (pScene)
    {
        std::vector<ECS::GameEntity*>& entities = pScene->getEntities();
        for (U64 i = 0; i < entities.size(); ++i)
        {
            std::tuple<Transform*> transformTuple = obtainTuple<Transform>(entities[i]->getUUID());
            Transform* transform = std::get<Transform*>(transformTuple);
            if (transform)
            {
                transform->updateMatrices();
                if (g_enableTransformLogging)
                {
                    ECS::GameEntity* entity = ECS::GameEntity::findEntity(transform->getOwner());
                    R_VERBOSE("Transform", "Owner: %s, Position: (%f, %f, %f)", entity->getName().c_str(), 
                        transform->position.x, transform->position.y, transform->position.z);
                }
            }
        }
    }
}


ResultCode TransformSystem::onCleanUp()
{
    return RecluseResult_Ok;
}
} // Recluse