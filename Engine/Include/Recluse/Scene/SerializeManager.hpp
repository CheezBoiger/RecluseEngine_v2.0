//
#include "Recluse/Types.hpp"

#include "Recluse/Game/GameEntity.hpp"
#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {
namespace Engine {

struct GameEntitySerializer 
{
    ECS::GameEntity* (*createGameEntity)();    
};


class SerializeManager 
{
public:

    static GameEntitySerializer* getObject();
    static void cacheSerializer(Hash64 hash, GameEntitySerializer& obj);
};
} // Engine
} // Recluse