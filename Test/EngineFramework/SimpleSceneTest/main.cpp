
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

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initialize();

    Engine::GameObject* obj = new TestObject();
    obj->initialize();  
    Scene* pScene = new Scene();    
    pScene->initialize();
    pScene->addGameObject(obj);

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