
#include <iostream>

#include "Recluse/Time.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/GameObject.hpp"

#include <vector>
#include <queue>

using namespace Recluse;
using namespace Recluse::Engine;

// Testing the game object behavior!
class TestObject : public ECS::GameObject {
public:
    R_PUBLIC_DECLARE_GAME_OBJECT(TestObject, "7C56ED72-A8C3-4428-952C-9BC7D4DB42C6")

    void onInitialize() override {
        m_name = "Super Test Object";
    }

    void onUpdate(const RealtimeTick& tick) override {
        R_TRACE(m_name.c_str(), "Testing this game object. Hello game!!");
    }
};


struct TestObject1 : public ECS::GameObject {
public:
    R_PUBLIC_DECLARE_GAME_OBJECT(TestObject1, "B8353C91-B1EC-4517-B7DC-0CBADB10A398")

    void onInitialize() override {
        m_name = "Normal Boring Object";
    }

    void onUpdate(const RealtimeTick& tick) override {
        R_VERBOSE(m_name.c_str(), "I am just a boring object... :(");
    }

    virtual ErrType serialize(Archive* pArchive) override {
        U64 szName = m_name.size();
        pArchive->write(&szName, sizeof(U64));
        pArchive->write(const_cast<char*>(m_name.c_str()), m_name.size());
        pArchive->write(&m_position, sizeof(Float2));
        return R_RESULT_OK;
    }

    virtual ErrType deserialize(Archive* pArchive) override {
        U64 szName = 0;
        pArchive->read(&szName, sizeof(U64));

        m_name.resize(szName);

        pArchive->read(const_cast<char*>(m_name.data()), szName);
        pArchive->read(&m_position, sizeof(Float2));
        return R_RESULT_OK;
    }

    void setPosition(const Float2& pos) { m_position = pos; }

private:
    Float2 m_position;
};


struct ChildObject : public ECS::GameObject {
public:
    R_PUBLIC_DECLARE_GAME_OBJECT(ChildObject, "E8C5D1C7-9758-4BF8-A17D-D1DC14D59769")

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


class GameWorldScene : public Engine::Scene {
public:
    ErrType serialize(Archive* pArchive) override {
        std::queue<ECS::GameObject*> objects;
        const std::vector<ECS::GameObject*>& gameObjects = getGameObjects();
        for (auto gameObject : gameObjects) {
            objects.push(gameObject);
        } 

        while (!objects.empty()) {
            ECS::GameObject* pObject = objects.front();
            objects.pop();
            
            ECS::RGUID rguid                                 = pObject->getClassGUID();
            const std::vector<ECS::GameObject*>& children    = pObject->getChildren();
            U32 numChildren                                     = (U32)children.size();
            const char* name                                    = pObject->getClassName();
            U32 nameSz                                          = strlen(name);

            pArchive->write(&rguid, sizeof(ECS::RGUID));
            pArchive->write(&numChildren, 4);
            pArchive->write(&nameSz, sizeof(U32));
            pArchive->write((void*)name, nameSz);
            
            pObject->serialize(pArchive);
        }

        return R_RESULT_OK;
    }

    ErrType deserialize(Archive* pArchive) override {

        ErrType result = R_RESULT_OK;
        while (result == R_RESULT_OK) {
            ECS::RGUID rguid;
            U32 numChilren  = 0;
            U32 nameSz      = 0;
            char name[128];
            result          = pArchive->read(&rguid, sizeof(ECS::RGUID));
            result          = pArchive->read(&numChilren, sizeof(U32));
            result          = pArchive->read(&nameSz, sizeof(U32));
            result          = pArchive->read(name, nameSz);
            name[nameSz]    = '\0';

            if (result == R_RESULT_OK) {
                R_TRACE(getName().c_str(), "Game Object: %s", name);
                
                if (rguid == TestObject::classGUID()) {
                    R_TRACE(__FUNCTION__, "This is a test object !");
                    TestObject* pTestObject = new TestObject();
                    pTestObject->deserialize(pArchive);
                    addGameObject(pTestObject);
                } else if (rguid == TestObject1::classGUID()) {
                    R_TRACE(__FUNCTION__, "This is a test object1 !");
                    TestObject1* pTestObject1 = new TestObject1();
                    pTestObject1->deserialize(pArchive);
                    addGameObject(pTestObject1);
                } else if (rguid == ChildObject::classGUID()) {
                    R_TRACE(__FUNCTION__, "This is a child object !");
                }
            }
        }

        return R_RESULT_OK;
    }
};

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);

    ECS::GameObject* obj = new TestObject();
    TestObject1* obj1 = new TestObject1();
    ECS::GameObject* child = new ChildObject();
    obj1->initialize();
    obj->initialize();  
    child->initialize();
    obj->addChild(child);
    
    obj1->setPosition(Float2(-5.0f, 32.0f));

    Scene* pScene = new GameWorldScene();
    pScene->setName("Scene Test...$#%@#^&*^*&^%$#qwiefowdnfOIEFONWLEKAFMPOWRHGJOIQNBER29-34U831634%#@%#$3\\[[");

    pScene->initialize();

    pScene->addGameObject(obj);
    pScene->addGameObject(obj1);

    U64 counter = 0;
    while ((counter++) < 500) {
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        pScene->update(tick);
    }

    Archive testArchive("Test.archive");

    testArchive.open("w");
    pScene->save(&testArchive);
    testArchive.close();

    obj->destroy();
    obj1->destroy();
    child->destroy();
    delete obj;
    delete obj1;
    delete child;
    pScene->destroy();

    testArchive.open("r");
    pScene->load(&testArchive);
    testArchive.close();    
    
    R_TRACE("TEST", "Scene Name: %s", pScene->getName().c_str());

    pScene->destroy();
  
    delete pScene;
    Log::destroyLoggingSystem();
    return 0;
}