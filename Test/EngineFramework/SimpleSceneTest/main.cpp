
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
#include <Windows.h>
#include <vector>

using namespace Recluse;
using namespace Recluse::Engine;

Recluse::MessageBus g_bus;

class AssertoHandler {
public:
    enum AssertResult {
        ASSERT_OK,
        ASSERT_DEBUG,
        ASSERT_IGNORE,
        ASSERT_STOP
    };
    static int check(Bool cond, const char* functionStr, const char* msg) {
        if (cond) {
            return ASSERT_OK;
        }
        std::string mstr;
        mstr += functionStr;
        mstr += "\n\n";
        mstr += msg;
        DWORD res = MessageBox(NULL, mstr.c_str(), NULL, MB_ABORTRETRYIGNORE);
        switch (res) {
            case IDRETRY: return ASSERT_DEBUG;
            default: return ASSERT_OK;
        }
    }
};

#define RE_ASSERT(cond, msg) {                                 \
    int _ = AssertoHandler::check(cond, __FUNCTION__, msg);                          \
    switch (_) { case AssertoHandler::ASSERT_DEBUG: DebugBreak(); break; default: break; } \
    }


enum InputEvents
{
    InputEvents_NOOB
};


const char* getEventString(EventId eventid)
{
    switch (eventid)
    {
        case InputEvents_NOOB:
            return "Noob";
        default:
            return "Error";
    }
}

class MoverComponent : public ECS::Component
{
public:
    R_COMPONENT_DECLARE(MoverComponent);

    MoverComponent() : ECS::Component(generateRGUID()) { }

    virtual void onRelease() override
    {
        MoverComponent::free(this);
    }
};


enum UpdaterEvent : U64
{
    UpdaterEvent_Update = 1232523423
};

class SimpleUpdaterSystem : public ECS::System<MoverComponent>
{
public:
    R_DECLARE_GAME_SYSTEM(SimpleUpdaterSystem(), MoverComponent);

    virtual ErrType onInitialize() override
    {
        g_bus.addReceiver("SimpleUpdaterSystem", [&](EventMessage* message) 
        {
             if (message->getEvent() == UpdaterEvent_Update)
                m_shouldUpdate = true;
        });
        return R_RESULT_OK;
    }

    virtual void onUpdateComponents(const RealtimeTick& tick) override 
    {
        if (m_shouldUpdate)
        {
            R_VERBOSE("SimpleUpdaterSystem", "Updating components...");
            for (auto& mover : m_movers)
            {
                if (mover->isEnabled())
                {
                    ECS::GameEntity* pEntity            = ECS::GameEntity::getEntity(mover->getOwner());
                    if (pEntity->isActive())
                    {       
                        Transform* t                        = pEntity->getComponent<Transform>();
                        ECS::System<Transform>* pSystemT    = ECS::castToSystem<Transform>();
                        t->position                         = t->position + Float3(1.0f, 0.f, 0.f) * tick.delta();

                        R_VERBOSE("SimpleUpdaterSystem", "Moving entity=%s, Position=(%f, %f, %f)", pEntity->getName().c_str(), t->position.x, t->position.y, t->position.z);
                    }
                }
            }
        }
        m_shouldUpdate = false;

        // Just testing fire events.
        MessageBus::fireEvent(&g_bus, UpdaterEvent_Update);
    }

    virtual ErrType onAllocateComponent(MoverComponent** pOut) override 
    {
        *pOut = new MoverComponent();
        m_movers.push_back(*pOut);
        // Just enable automatically.
        m_movers.back()->setEnable(true);
        return R_RESULT_OK;
    }

    virtual ErrType onAllocateComponents(MoverComponent*** pOuts, U32 count) override { return R_RESULT_NO_IMPL; }
    virtual ErrType onFreeComponent(MoverComponent** pIn) override { if (*pIn) delete *pIn; return R_RESULT_OK; }
    virtual ErrType onFreeComponents(MoverComponent*** pIns, U32 count) override { return R_RESULT_NO_IMPL; }
private:
    std::vector<MoverComponent*> m_movers;
    Bool m_shouldUpdate = true;
};

R_COMPONENT_IMPLEMENT(MoverComponent, SimpleUpdaterSystem);


int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);
    g_bus.initialize();

    Transform::systemInit();
    MoverComponent::systemInit();

    ECS::GameEntity* entity = ECS::GameEntity::instantiate(sizeof(ECS::GameEntity));
    entity->setName("Billy");
    entity->activate();
    entity->addComponent<Transform>();
    entity->addComponent<MoverComponent>();
    Scene* pScene = new Scene();
    pScene->initialize();
    pScene->addEntity(entity);
    pScene->registerSystem(Transform::getSystem());
    pScene->registerSystem(MoverComponent::getSystem());

    U64 counter = 0;
    while ((counter++) < 500) {

        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        pScene->update(tick);

        g_bus.notifyAll();
        g_bus.clearQueue();
    }

    pScene->destroy();
    delete pScene;
    Log::destroyLoggingSystem();
    g_bus.cleanUp();
    return 0;
}