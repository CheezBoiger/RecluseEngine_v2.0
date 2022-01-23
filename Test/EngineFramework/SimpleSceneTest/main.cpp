
#include <iostream>

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/GameObject.hpp"
#include "Recluse/MessageBus.hpp"
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

struct InputMessage : public AMessage {
  std::string getEvent() override { return "Noob"; }
  U32 value;
};


// Testing the game object behavior!
class TestObject : public Engine::GameObject {
public:
    R_PUBLIC_DECLARE_GAME_OBJECT(TestObject, "7C56ED72-A8C3-4428-952C-9BC7D4DB42C6")

    void onInitialize() override {
        m_name = "Super Test Object";
      std::function<void(AMessage*)> fun = [&](AMessage* message) {
          InputMessage* input = static_cast<InputMessage*>(message);
          R_VERBOSE(m_name.c_str(), "Message: %s, value=%d", input->getEvent().c_str(), input->value);
      }; 
      g_bus.addReceiver("TestObject", fun);
    }

    void onUpdate(const RealtimeTick& tick) override {
        R_TRACE(m_name.c_str(), "Testing this game object. Hello game!!");
    }
};


struct TestObject1 : public Engine::GameObject {
public:
    R_PUBLIC_DECLARE_GAME_OBJECT(TestObject1, "B8353C91-B1EC-4517-B7DC-0CBADB10A398")

    void onInitialize() override {
        m_name = "Normal Boring Object";
    }

    void onUpdate(const RealtimeTick& tick) override {
        R_VERBOSE(m_name.c_str(), "I am just a boring object... :(");
    }
};


struct ChildObject : public Engine::GameObject {
public:
    R_PUBLIC_DECLARE_GAME_OBJECT(ChildObject, "E8C5D1C7-9758-4BF8-A17D-D1DC14D59769")

    void onInitialize() override {
        m_name = "Child Object";
    }

    void onUpdate(const RealtimeTick& tick) override {
        GameObject* parent = getParent();
        InputMessage message = { };
        message.value = 70;
        g_bus.pushMessage(message);
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
    RealtimeTick::initializeWatch(1ull, 0);
    g_bus.initialize();

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
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        pScene->update(tick);

        g_bus.notifyAll();
        g_bus.clearQueue();
    }

    pScene->destroy();
    delete obj;  
    delete pScene;
    Log::destroyLoggingSystem();
    g_bus.cleanUp();
    return 0;
}