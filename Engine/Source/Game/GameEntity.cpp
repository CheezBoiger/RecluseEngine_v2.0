//
#include "Recluse/Game/GameEntity.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Archive.hpp"

namespace Recluse {
namespace ECS {


static GameEntity* defaultAlloc(U64 szBytes, GameEntityMemoryAllocationType type)
{
    GameEntityAllocation allocation = { };

    allocation.offsetAddress    = (SizeT)malloc(szBytes);
    allocation.szBytes          = szBytes;
    allocation.allocType        = type;

    // A probably not so good way of keeping the allocation, but it is indeed default.
    RGUID guid      = { };
    guid.ss.hash0   = (U32)((allocation.offsetAddress & 0x00000000FFFFFFFF)      );
    guid.ss.hash1   = (U32)((allocation.offsetAddress & 0xFFFFFFFF00000000) >> 32);
    
    void* ptr       = reinterpret_cast<void*>(allocation.offsetAddress);

    return new (ptr) GameEntity(allocation, guid);
}

static void defaultFree(GameEntity* pEntity)
{
    R_ASSERT(pEntity != NULL);

    GameEntityAllocation allocation = pEntity->getAllocation();
    
    free((void*)allocation.offsetAddress);
}


static GameEntity* defaultGetEntity(const RGUID& rguid)
{
    // Get the rguid hash.
    SizeT address = U64(rguid.ss.hash0) | (U64(rguid.ss.hash1) << 32);
    GameEntity* pEntity = reinterpret_cast<GameEntity*>(address);
    return pEntity;
}


GameEntityAllocationCall gameEntityAllocator { nullptr, nullptr, defaultAlloc, defaultFree, defaultGetEntity };


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


GameEntity* GameEntity::getEntity(const RGUID& guid)
{
    return gameEntityAllocator.onGetEntityByRguidFn(guid);
}


ErrType GameEntity::serialize(Archive* pArchive)
{
    // TODO: Need to figure out how to obtain a proper rguid.
    //       Parent needs to also be included as well!
    RGUID guid = getUUID();
    pArchive->write(&guid, sizeof(RGUID));

    const std::string& s    = getName();
    U64 szBytes             = s.size();
    const char* str         = s.c_str();

    pArchive->write((void*)str, szBytes);

    const std::string tag   = getTag();
    szBytes                 = tag.size();
    str                     = tag.c_str();

    pArchive->write((void*)str, szBytes);

    return R_RESULT_OK;
}

ErrType GameEntity::deserialize(Archive* pArchive)
{
    return R_RESULT_NO_IMPL;
}
} // ECS
} // Recluse 