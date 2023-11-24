
#include <iostream>

#include "Recluse/Time.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/MessageBus.hpp"
#include "Recluse/Game/Components/Transform.hpp"
#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/Math/Bounds2D.hpp"
#include <Windows.h>
#include <vector>

using namespace Recluse;
using namespace Recluse::Engine;

Recluse::MessageBus g_bus;

class MoverComponent : public ECS::Component<MoverComponent>
{
public:
    R_COMPONENT_DECLARE(MoverComponent);
    Math::Float2 direction;
    Math::Bounds2d bounds = { Math::Float2(-1, -1), Math::Float2(1, 1) };
    F32 damage = 0;
    MoverComponent() : ECS::Component<MoverComponent>(generateRGUID()) { }
};


enum UpdaterEvent : U64
{
    UpdaterEvent_Update = 1232523423
};

class SimpleUpdaterSystem : public ECS::System<MoverComponent>
{
public:
    R_DECLARE_GAME_SYSTEM(SimpleUpdaterSystem, MoverComponent);

    virtual ResultCode onInitialize() override
    {
        g_bus.addReceiver("SimpleUpdaterSystem", [&](EventMessage* message) 
        {
             if (message->getEvent() == UpdaterEvent_Update)
                m_shouldUpdate = true;
        });
        return RecluseResult_Ok;
    }

    virtual void onUpdateComponents(const RealtimeTick& tick) override 
    {
        if (m_shouldUpdate)
        {
            for (auto& mover : m_movers)
            {
                if (mover->isEnabled())
                {
                    ECS::GameEntity* pEntity            = ECS::GameEntity::findEntity(mover->getOwner());
                    if (pEntity->isActive())
                    {       
                        Transform* t                        = pEntity->getComponent<Transform>();
                        ECS::System<Transform>* pSystemT    = ECS::castToSystem<Transform>();
                        t->position                         = t->position + Float3(mover->direction, 0.f) * tick.delta();

                        //R_VERBOSE("SimpleUpdaterSystem", "Moving entity=%s, Position=(%f, %f, %f)", pEntity->getName().c_str(), t->position.x, t->position.y, t->position.z);
                    }
                }
            }
        }
        m_shouldUpdate = false;

        // Just testing fire events.
        MessageBus::fireEvent(&g_bus, UpdaterEvent_Update);
    }

    virtual ResultCode onAllocateComponent(MoverComponent** pOut) override 
    {
        *pOut = new MoverComponent();
        m_movers.push_back(*pOut);
        // Just enable automatically.
        m_movers.back()->setEnable(true);
        return RecluseResult_Ok;
    }

    virtual ResultCode onAllocateComponents(MoverComponent*** pOuts, U32 count) override { return RecluseResult_NoImpl; }
    virtual ResultCode onFreeComponent(MoverComponent** pIn) override { if (*pIn) delete *pIn; return RecluseResult_Ok; }
    virtual ResultCode onFreeComponents(MoverComponent*** pIns, U32 count) override { return RecluseResult_NoImpl; }

    virtual ResultCode onCleanUp() override 
    {
        for (auto it = m_movers.begin(); it != m_movers.end(); ++it)
            delete (*it);
        m_movers.clear();
        return RecluseResult_Ok; 
    }

private:
    std::vector<MoverComponent*> m_movers;
    Bool m_shouldUpdate = true;
};


// Health component
struct HealthComponent : public ECS::Component<HealthComponent>
{
    R_COMPONENT_DECLARE(HealthComponent);
    F32 m_health = 100.f;
    RGUID other;
    HealthComponent() : ECS::Component<HealthComponent>(generateRGUID()) { }
};


class HealthSystem : public ECS::System<HealthComponent>
{
public:
    R_DECLARE_GAME_SYSTEM(HealthSystem, HealthComponent);
    virtual ResultCode onAllocateComponent(HealthComponent** pOut) override 
    { 
        *pOut = new HealthComponent();
        m_components.push_back(*pOut);
        m_components.back()->setEnable(true);
        return RecluseResult_Ok; 
    }

    virtual ResultCode onAllocateComponents(HealthComponent*** pOuts, U32 count) override { return RecluseResult_NoImpl; }
    virtual ResultCode onFreeComponent(HealthComponent** pIn) override { return RecluseResult_NoImpl; }
    virtual ResultCode onFreeComponents(HealthComponent*** pOuts, U32 count) override { return RecluseResult_NoImpl; }

    virtual ResultCode onCleanUp() override
    {
        for (auto it = m_components.begin(); it != m_components.end(); ++it)
            delete (*it);
        m_components.clear();
        return RecluseResult_Ok;
    }

    virtual void onUpdateComponents(const RealtimeTick& tick) override 
    {
        for (auto& healthComponent : m_components)
        {
            ECS::GameEntity* entity = ECS::GameEntity::findEntity(healthComponent->getOwner());
            ECS::GameEntity* otherEntity = ECS::GameEntity::findEntity(healthComponent->other);
            if (otherEntity)
            {
                MoverComponent* mover = entity->getComponent<MoverComponent>();
                MoverComponent* otherMover = otherEntity->getComponent<MoverComponent>();
                Transform* transforms[] = { entity->getComponent<Transform>(), otherEntity->getComponent<Transform>() };
                Math::Bounds2d bounds[] = { mover->bounds, otherMover->bounds };

                for (U32 i = 0; i < 2; ++i)
                {
                    bounds[i].mmin = bounds[i].mmin + Math::Float2(transforms[i]->position.x, transforms[i]->position.y);
                    bounds[i].mmax = bounds[i].mmax + Math::Float2(transforms[i]->position.x, transforms[i]->position.y);
                }

                if (Math::intersects(bounds[0], bounds[1]))
                {
                    healthComponent->m_health -= tick.delta() * otherMover->damage;
                    healthComponent->m_health = Math::maximum(healthComponent->m_health, 0.0f);
                    R_VERBOSE("HealthSystem", "%s is taking %f damage! Health=%f", entity->getName().c_str(), otherMover->damage, healthComponent->m_health);
                    if (healthComponent->m_health == 0.f)
                        R_ERROR("HealthSystem", "%s IS DEAD!!!", entity->getName().c_str());
                }
            }
        }
    }
private:
    std::vector<HealthComponent*> m_components;
};

R_BIND_COMPONENT_SYSTEM(MoverComponent, SimpleUpdaterSystem);
R_BIND_COMPONENT_SYSTEM(HealthComponent, HealthSystem);


int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);
    g_bus.initialize();

    Transform::systemInit();
    MoverComponent::systemInit();
    HealthComponent::systemInit();

    ECS::GameEntity* entity = ECS::GameEntity::instantiate(sizeof(ECS::GameEntity));
    ECS::GameEntity* entity2 = ECS::GameEntity::instantiate(sizeof(ECS::GameEntity));
    entity->setName("Billy");
    entity->activate();
    entity->addComponent<Transform>();
    entity->addComponent<MoverComponent>();
    entity->addComponent<HealthComponent>();
    entity->getComponent<MoverComponent>()->direction = Math::Float2(1, 0);
    entity->getComponent<MoverComponent>()->damage = 10.0f;
    entity->getComponent<Transform>()->position = Math::Float3(-5, 0, 0);
    entity->getComponent<HealthComponent>()->other = entity2->getUUID();

    entity2->setName("Alice");
    entity2->activate();
    entity2->addComponent<Transform>();
    entity2->addComponent<MoverComponent>();
    entity2->addComponent<HealthComponent>();
    entity2->getComponent<MoverComponent>()->direction = Math::Float2(-1, 0);
    entity2->getComponent<Transform>()->position = Math::Float3(5, 0, 0);
    entity2->getComponent<HealthComponent>()->other = entity->getUUID();
    entity2->getComponent<MoverComponent>()->damage = 54.0f;

    Scene* pScene = new Scene();
    pScene->initialize();
    pScene->addEntity(entity);
    pScene->addEntity(entity2);
    pScene->addSystemFor<Transform>();
    pScene->addSystemFor<MoverComponent>();
    pScene->addSystemFor<HealthComponent>();

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