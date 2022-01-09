//
#include "Recluse/Types.hpp"

#include "Recluse/Game/GameObject.hpp"
#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {
namespace Engine {

struct GameObjectSerializer 
{
    GameObject* (*createGameObject)();    
};


class SerializeManager 
{
public:

    static GameObjectSerializer* getObject();
    static void cacheSerializer(Hash64 hash, GameObjectSerializer obj);
};
} // Engine
} // Recluse