//
#include "Recluse/Game/GameEntity.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace ECS {


GameEntity* defaultAlloc(U64 szBytes, GameEntityMemoryAllocationType type)
{
    GameEntityAllocation allocation;
    return new GameEntity(allocation, generateRGUID());
}

void defaultFree(GameEntity* pEntity)
{
}


GameEntityAllocationCall gameEntityAllocator { defaultAlloc, defaultFree };


GameEntity* GameEntity::instantiate(U64 szBytes, GameEntityMemoryAllocationType allocType)
{
    return gameEntityAllocator.onAllocationFn(szBytes, allocType);
}


void GameEntity::free(GameEntity* entity)
{
    gameEntityAllocator.onFreeFn(entity);
}


void GameEntity::setOnAllocation(GameEntityAllocationCall callback)
{
    R_ASSERT(callback.onAllocationFn != nullptr || callback.onFreeFn != nullptr);
    gameEntityAllocator = callback;
}
} // ECS
} // Recluse 