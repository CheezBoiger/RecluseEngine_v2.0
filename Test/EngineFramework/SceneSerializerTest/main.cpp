
#include <iostream>

#include "Recluse/Time.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Game/Components/Transform.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/GameEntity.hpp"

#include <vector>
#include <queue>

using namespace Recluse;
using namespace Recluse::Engine;

class GameWorldScene : public Engine::Scene {
public:
    ErrType serialize(Archive* pArchive) override {
        std::queue<ECS::GameEntity*> objects;
        const std::vector<ECS::GameEntity*>& gameObjects = getEntities();
        for (auto gameObject : gameObjects) {
            objects.push(gameObject);
        } 

        while (!objects.empty()) {
            ECS::GameEntity* pObject = objects.front();
            objects.pop();
            
            RGUID rguid                                 = pObject->getUUID();
            const std::vector<ECS::GameEntity*>& children    = pObject->getChildren();
            U32 numChildren                                     = (U32)children.size();
            const char* name                                    = "cats";
            U32 nameSz                                          = strlen(name);

            pArchive->write(&rguid, sizeof(RGUID));
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
            RGUID rguid;
            U32 numChilren  = 0;
            U32 nameSz      = 0;
            char name[128];
            result          = pArchive->read(&rguid, sizeof(RGUID));
            result          = pArchive->read(&numChilren, sizeof(U32));
            result          = pArchive->read(&nameSz, sizeof(U32));
            result          = pArchive->read(name, nameSz);
            name[nameSz]    = '\0';

            if (result == R_RESULT_OK) {
            }
        }

        return R_RESULT_OK;
    }
};

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);

    Scene* pScene = new GameWorldScene();
    pScene->setName("Scene Test...$#%@#^&*^*&^%$#qwiefowdnfOIEFONWLEKAFMPOWRHGJOIQNBER29-34U831634%#@%#$3\\[[");

    pScene->initialize();

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