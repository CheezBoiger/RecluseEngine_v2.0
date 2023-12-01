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
        U64 count = 0;
        Transform** transforms = pScene->getRegistry<Transform>()->getAllComponents(count);
        for (U64 i = 0; i < count; ++i)
        {
            transforms[i]->updateMatrices();
            if (g_enableTransformLogging)
            {
                ECS::GameEntity* entity = ECS::GameEntity::findEntity(transforms[i]->getOwner());
                R_VERBOSE("Transform", "Owner: %s, Position: (%f, %f, %f)", entity->getName().c_str(), 
                    transforms[i]->position.x, transforms[i]->position.y, transforms[i]->position.z);
            }
        }
    }
}


ResultCode TransformSystem::onCleanUp()
{
    return RecluseResult_Ok;
}
} // Recluse