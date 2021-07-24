
#include <iostream>

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/GameObject.hpp"

#include <vector>

using namespace Recluse;
using namespace Recluse::Engine;

// Testing the game object behavior!
class TestObject : public Engine::GameObject {
public:
    void onInitialize() override {
        m_name = "Super Test Object";
    }

    void onUpdate(const RealtimeTick& tick) override {
        R_TRACE(m_name.c_str(), "Testing this game object. Hello game!!");
    }
};


struct TestObject1 : public Engine::GameObject {
public:
    void onInitialize() override {
        m_name = "Normal Boring Object";
    }

    void onUpdate(const RealtimeTick& tick) override {
        R_VERBOSE(m_name.c_str(), "I am just a boring object... :(");
    }
};


struct ChildObject : public Engine::GameObject {
public:
    void onInitialize() override {
        m_name = "Child Object";
    }

    void onUpdate(const RealtimeTick& tick) override {
        GameObject* parent = getParent();
        if (parent) {
            R_WARN(m_name.c_str(), "I am a child object! My father is %s!", parent->getName().c_str());
        } else {
            R_ERR(m_name.c_str(), "I am a child object, but I don't have a parent!!");
        }
    }
};

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initialize();

    Engine::GameObject* obj = new TestObject();
    Engine::GameObject* obj1 = new TestObject1();
    Engine::GameObject* child = new ChildObject();
    obj1->initialize();
    obj->initialize();  
    child->initialize();
    obj->addChild(child);
    Scene* pScene = new Scene();    
    pScene->initialize();
    pScene->addGameObject(obj);
    pScene->addGameObject(obj1);

    U64 counter = 0;
    while ((counter++) < 500) {
        RealtimeTick tick = RealtimeTick::getTick();
        pScene->update(tick);
    }

    pScene->destroy();

    delete obj;  
    delete pScene;
    Log::destroyLoggingSystem();
    return 0;
}