//
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/Systems/TransformSystem.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Application.hpp"
#include "Recluse/Generated/Game/TranformEvents.hpp"

namespace Recluse {


R_DECLARE_GLOBAL_BOOLEAN(g_enableTransformLogging, false, "Transform.EnableLogging");


ResultCode TransformSystem::onInitialize(MessageBus* bus)
{
    if (bus)
    {
        bus->addReceiver("TransformSystem", 
            [&] (EventMessage* msg) -> void 
            {
                if (msg->getEvent() == TransformEvent_Update)
                    m_doUpdate = true;
            });
    }
    return RecluseResult_Ok;
}


void TransformSystem::onUpdate(const RealtimeTick& tick)
{
    Engine::Scene* pScene = getScene();
    if (pScene && m_doUpdate)
    {
        const std::vector<ECS::GameEntity*>& entities = pScene->getEntities();
        for (U64 i = 0; i < entities.size(); ++i)
        {
            ECS::GameEntity* entity = entities[i];
            std::tuple<Transform*> transformTuple = obtainTuple<Transform>(entity->getUUID());
            Transform* transform = std::get<Transform*>(transformTuple);
            if (transform)
            {
                Transform* parentTransform = std::get<Transform*>(obtainTuple<Transform>(entity->getParent()));
                transform->updateMatrices(parentTransform);
                if (g_enableTransformLogging)
                {
                    ECS::GameEntity* tentity = ECS::GameEntity::findEntity(transform->getOwner());
                    R_VERBOSE("Transform", "Owner: %s, Position: (%f, %f, %f)", tentity->getName().c_str(), 
                        transform->position.x, transform->position.y, transform->position.z);
                }
            }
        }
        m_doUpdate = false;
    }
}


ResultCode TransformSystem::onCleanUp()
{
    return RecluseResult_Ok;
}
} // Recluse