
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
    R_DECLARE_COMPONENT(MoverComponent);
    Math::Float3 direction;
};


class MoverEvent : public EventMessage
{
public:
    U32 component;
    MoverEvent(EventId eventId, U32 componentId)
        : component(component) { }
};


class MoverRegistry : public ECS::ComponentRegistry<MoverComponent>
{
public:
    R_DECLARE_COMPONENT_REGISTRY(MoverRegistry);
    
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

    U32 queryComponents(MoverComponent::Pointer* outComponents, U32 count) override
    {
        if (outComponents)
        {
            for (U32 i = 0; i < components.size(); ++i)
                outComponents[i] = components[i];
        }
        return static_cast<U32>(components.size());
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

    ResultCode onInitialize() override
    {
        return RecluseResult_Ok;
    }

    void onUpdate(ECS::Registry* registry, const RealtimeTick& tick, ECS::EntityHierarchy* hierarchy) override
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
        MessageBus::sendEvent(&g_bus, TransformEvent_Update);
        MessageBus::sendEvent<MoverEvent>(&g_bus, MovementEventId_DoMovement, 12);
    }

    ResultCode onEvent(const EventMessage& message) override
    {
        switch (message.getEvent())
        {
            case MovementEventId_DoMovement:
            {
                const MoverEvent& event = EventMessage::castTo<MoverEvent>(message);
                break;
            }
        }

        return RecluseResult_Ok;
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

    pScene->addEntity(entity->getGUID());
    pScene->addEntity(entity2->getGUID());

    pScene->getHierarchy()->add(entity->getGUID());
    pScene->getHierarchy()->add(entity2->getGUID(), entity->getGUID());

    registry->makeComponent<Transform>(entity->getGUID(), true);
    registry->makeComponent<Transform>(entity2->getGUID(), true);

    registry->makeComponent<MoverComponent>(entity->getGUID(), true);
    GlobalCommands::setValue("Transform.EnableLogging", true);

    registry->getComponent<Transform>(entity->getGUID())->position = Math::Float3(43, 12, -2);
    registry->getComponent<MoverComponent>(entity->getGUID())->direction = Math::normalize(Math::Float3(1, 0, 0));

    registry->makeComponent<MoverComponent>(entity2->getGUID(), true);
    registry->getComponent<MoverComponent>(entity2->getGUID())->direction = Math::normalize(Math::Float3(-1, 0, 0));
}


int main(int c, char* argv[])
{
    LogSystem::initializeLoggingSystem();
    LogSystem::enableLogTypes(LogType_Verbose);
    RealtimeTick::initializeWatch(1ull, 0);
    g_bus.initialize();

    ECS::Registry registry;
    Scene* pScene = new Scene();
    pScene->initialize();

    ECS::AbstractSystem* transformSystem = ECS::AbstractSystem::allocate<TransformSystem>();
    ECS::AbstractSystem* moverSystem = ECS::AbstractSystem::allocate<MoverSystem>();

    addEntities(pScene, &registry);

    transformSystem->linkMessageBus(&g_bus);
    moverSystem->linkMessageBus(&g_bus);

    F32 counter = 0;
    while (counter < 10.0f) 
    {
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        moverSystem->update(&registry, tick, pScene->getHierarchy());
        transformSystem->update(&registry, tick, pScene->getHierarchy());
        g_bus.notifyAll();
        g_bus.clearQueue();
        counter += tick.delta() * 1.0f;
        pollEvents();
    }

    pScene->destroy();
    delete pScene;
    transformSystem->cleanUp();
    moverSystem->cleanUp();
    ECS::AbstractSystem::free(transformSystem);
    ECS::AbstractSystem::free(moverSystem);
    registry.cleanUp();
    LogSystem::destroyLoggingSystem();
    g_bus.cleanUp();
    return 0;
}