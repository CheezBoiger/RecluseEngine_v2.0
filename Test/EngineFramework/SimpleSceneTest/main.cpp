
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
#include <Windows.h>
#include <vector>

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


class MoverRegistry : public ECS::Registry<MoverComponent>
{
public:
    R_COMPONENT_REGISTRY_DECLARE(MoverRegistry);
    
    ResultCode onAllocateComponent(const RGUID& owner) override
    {
        MoverComponent* comp = new MoverComponent();
        comp->setOwner(owner);
        components.push_back(comp);
        return RecluseResult_Ok;
    }

    ResultCode onFreeComponent(const RGUID& owner) override
    {
        for (auto it = components.begin(); it != components.end(); ++it)
        {
            if (owner == (*it)->getOwner())
            {
                delete (*it);
                components.erase(it);
                return RecluseResult_Ok;
            }
        }
        return RecluseResult_NotFound;
    }

    MoverComponent** getAllComponents(U64& count) override
    {
        count = components.size();
        return components.data();
    }

    MoverComponent* getComponent(const RGUID& owner) override
    {
        for (auto it = components.begin(); it != components.end(); ++it)
        {
            if (owner == (*it)->getOwner())
            {
                return (*it);
            }
        }
        return nullptr;
    }

private:
    std::vector<MoverComponent*> components;
};


class MoverSystem : public ECS::System
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

    void onUpdate(const RealtimeTick& tick) override
    {
        std::vector<ECS::GameEntity*> entities = getScene()->getEntities();
        for (U64 i = 0; i < entities.size(); ++i)
        {
            std::tuple<MoverComponent*, Transform*> tp = obtainTuple<MoverComponent, Transform>(entities[i]->getUUID());
            MoverComponent* mover = std::get<MoverComponent*>(tp);
            Transform* transform = std::get<Transform*>(tp);
            if (mover && transform)
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


void addEntities(Scene* pScene)
{
    ECS::GameEntity* entity = ECS::GameEntity::instantiate(sizeof(ECS::GameEntity));
    ECS::GameEntity* entity2 = ECS::GameEntity::instantiate(sizeof(ECS::GameEntity));
    entity->setName("Billy");
    entity->activate();

    entity2->setName("Alice");
    entity2->activate();

    pScene->addEntity(entity);
    pScene->addEntity(entity2);

    pScene->addComponentForEntity<Transform>(entity->getUUID());
    pScene->addComponentForEntity<Transform>(entity2->getUUID());

    pScene->addComponentForEntity<MoverComponent>(entity->getUUID());
    GlobalCommands::setValue("Transform.EnableLogging", true);

    entity->getComponent<Transform>(pScene)->position = Math::Float3(43, 12, -2);
    entity->getComponent<MoverComponent>(pScene)->direction = Math::normalize(Math::Float3(1, 0, 0));

    pScene->addComponentForEntity<MoverComponent>(entity2->getUUID());
    entity2->getComponent<MoverComponent>(pScene)->direction = Math::normalize(Math::Float3(-1, 0, 0));
}


int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    enableLogTypes(LogType_Verbose);
    RealtimeTick::initializeWatch(1ull, 0);
    g_bus.initialize();

    Scene* pScene = new Scene();
    pScene->initialize();
    pScene->addRegistry<TransformRegistry>();
    pScene->addRegistry<MoverRegistry>();
    pScene->addSystem<TransformSystem>(&g_bus);
    pScene->addSystem<MoverSystem>(&g_bus);

    addEntities(pScene);

    F32 counter = 0;
    while (counter < 10.0f) {

        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        pScene->update(tick);

        g_bus.notifyAll();
        g_bus.clearQueue();
        counter += tick.delta() * 1.0f;
    }

    pScene->destroy();
    delete pScene;
    Log::destroyLoggingSystem();
    g_bus.cleanUp();
    return 0;
}