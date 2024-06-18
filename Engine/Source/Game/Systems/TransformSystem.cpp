//
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/Systems/TransformSystem.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Application.hpp"
#include "Recluse/Generated/Game/TranformEvents.hpp"

#include "Recluse/Game/Components/Camera.hpp"
#include <vector>

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


void TransformSystem::onUpdate(ECS::Registry* registry, const RealtimeTick& tick)
{
    if (m_doUpdate)
    {
        std::vector<Transform*> transforms = obtainComponents(registry);
        for (U64 i = 0; i < transforms.size(); ++i)
        {
            Transform* transform = transforms[i];

            // Check if the transform is disabled.
            if (!transform->isEnabled())
                continue;

            ECS::GameEntity* entity = ECS::GameEntity::findEntity(transform->getOwner());
            // Get the parent transform and transform locally on it.
            Transform* parentTransform = std::get<Transform*>(obtainTuple<Transform>(registry, entity->getParent()));
            transform->updateMatrices(parentTransform);
            if (g_enableTransformLogging)
            {
                ECS::GameEntity* tentity = ECS::GameEntity::findEntity(transform->getOwner());
                R_VERBOSE("Transform", "Owner: %s, Position: (%f, %f, %f)", tentity->getName().c_str(), 
                    transform->position.x, transform->position.y, transform->position.z);
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