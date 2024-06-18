
#include <iostream>

#include "Recluse/Time.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/MessageBus.hpp"
#include "Recluse/Game/Components/Transform.hpp"
#include "Recluse/Game/Systems/TransformSystem.hpp"
#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Math/Bounds2D.hpp"
#include "Recluse/System/KeyboardInput.hpp"
#include "Recluse/System/Input.hpp"
#include <Windows.h>
#include <vector>
#include <unordered_map>

#include "Recluse/Generated/Game/TranformEvents.hpp"

using namespace Recluse;
using namespace Recluse::Engine;

Recluse::MessageBus g_bus;


enum MovementEventId
{
    MovementEventId_DoMovement = 23510342
};

class MoverComponent : public ECS::Component
{
public:
    R_COMPONENT_DECLARE(MoverComponent);
    Math::Float3 direction;
};


class MoverRegistry : public ECS::ComponentRegistry<MoverComponent>
{
public:
    R_COMPONENT_REGISTRY_DECLARE(MoverRegistry);
    
    ResultCode onAllocateComponent(const RGUID& owner) override
    {
        auto iter = m_map.find(owner);
        if (iter == m_map.end())
        {
            MoverComponent* comp = new MoverComponent();
            comp->setOwner(owner);
            components.push_back(comp);
            m_map.insert(std::make_pair(owner, comp));
        }
        else
        {
            return RecluseResult_Failed;
        }

        return RecluseResult_Ok;
    }

    ResultCode onFreeComponent(const RGUID& owner) override
    {
        auto iter = m_map.find(owner);
        if (iter != m_map.end())
        {
            if (owner == iter->second->getOwner())
            {
                delete iter->second;
                m_map.erase(iter);
                for (auto it = components.begin(); it != components.end(); ++it)
                {
                    if (owner == (*it)->getOwner())
                    {
                        components.erase(it);
                        break;
                    }
                }
                return RecluseResult_Ok;
            }
        }
        return RecluseResult_NotFound;
    }

    std::vector<MoverComponent*> getAllComponents() override
    {
        std::vector<MoverComponent*> components(components);
        return components;
    }

    MoverComponent* getComponent(const RGUID& owner) override
    {
        auto it = m_map.find(owner);
        if (it != m_map.end())
        {
            return it->second;
        }
        return nullptr;
    }

private:
    std::vector<MoverComponent*> components;
    std::unordered_map<RGUID, MoverComponent*, RGUID::Hash> m_map;
};


class MoverSystem : public ECS::System<MoverComponent>
{
public:
    R_DECLARE_GAME_SYSTEM(MoverSystem);

    ResultCode onInitialize(MessageBus* bus) override
    {
        if (bus)
        {
            bus->addReceiver("MoverSystem", [] (EventMessage* msg) -> void 
            {
                msg->getEvent(); 
            });
        }
        return RecluseResult_Ok;
    }

    void onUpdate(ECS::Registry* registry, const RealtimeTick& tick) override
    {
        std::vector<MoverComponent*> movers = obtainComponents(registry);
        for (U64 i = 0; i < movers.size(); ++i)
        {
            MoverComponent* mover = movers[i];
            std::tuple<Transform*> tp = obtainTuple<Transform>(registry, mover->getOwner());
            Transform* transform = std::get<Transform*>(tp);
            if (transform)
            {
                transform->position = transform->position + mover->direction * tick.delta();
            }
        }
        MessageBus::fireEvent(&g_bus, TransformEvent_Update);
    }

    ResultCode onCleanUp() override
    {
        return RecluseResult_Ok;
    }
};


void addEntities(Scene* pScene, ECS::Registry* registry)
{
    // Add in registries for components.
    registry->addComponentRegistry<TransformRegistry>();
    registry->addComponentRegistry<MoverRegistry>();

    ECS::GameEntity* entity = ECS::GameEntity::instantiate(sizeof(ECS::GameEntity));
    ECS::GameEntity* entity2 = ECS::GameEntity::instantiate(sizeof(ECS::GameEntity));
    entity->setName("Billy");
    entity->activate();

    entity2->setName("Alice");
    entity2->activate();

    pScene->addEntity(entity);
    pScene->addEntity(entity2);

    registry->makeComponent<Transform>(entity->getUUID(), true);
    registry->makeComponent<Transform>(entity2->getUUID(), true);

    registry->makeComponent<MoverComponent>(entity->getUUID(), true);
    GlobalCommands::setValue("Transform.EnableLogging", true);

    entity->getComponent<Transform>(registry)->position = Math::Float3(43, 12, -2);
    entity->getComponent<MoverComponent>(registry)->direction = Math::normalize(Math::Float3(1, 0, 0));

    registry->makeComponent<MoverComponent>(entity2->getUUID(), true);
    entity2->getComponent<MoverComponent>(registry)->direction = Math::normalize(Math::Float3(-1, 0, 0));
}


int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    enableLogTypes(LogType_Verbose);
    RealtimeTick::initializeWatch(1ull, 0);
    g_bus.initialize();

    ECS::Registry registry;
    Scene* pScene = new Scene();
    pScene->initialize();
    pScene->addSystem<TransformSystem>(&g_bus);
    pScene->addSystem<MoverSystem>(&g_bus);

    addEntities(pScene, &registry);

    F32 counter = 0;
    while (counter < 10.0f) {

        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        pScene->update(&registry, tick);

        g_bus.notifyAll();
        g_bus.clearQueue();
        counter += tick.delta() * 1.0f;
        pollEvents();
    }

    pScene->destroy();
    delete pScene;
    registry.cleanUp();
    Log::destroyLoggingSystem();
    g_bus.cleanUp();
    return 0;
}