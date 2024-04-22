//
#include "Recluse/Game/GameEntity.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Archive.hpp"

namespace Recluse {
namespace ECS {


static GameEntity* defaultAlloc(U64 szBytes, GameEntityMemoryAllocationType type)
{
    GameEntityAllocation allocation = { };

    R_ASSERT(szBytes == sizeof(GameEntity));

    if (type == GameEntityMemoryAllocationType_Dynamic)
    {
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
    else
    {
        R_ERROR("GameEntity", "Only Dynamic allocation is supported for default allocations!");
    }

    // Return null if we can't allocate the specified allocation type.
    return nullptr;
}

static void defaultFree(GameEntity* pEntity)
{
    R_ASSERT(pEntity != NULL);

    GameEntityAllocation allocation = pEntity->getAllocation();
    
    free((void*)allocation.offsetAddress);
}


static GameEntity* defaultGetEntity(const RGUID& rguid)
{
    // Invalid guid, will return nullptr.
    if (rguid == RGUID::kInvalidValue)
        return nullptr;

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


GameEntity* GameEntity::findEntity(const RGUID& guid)
{
    return gameEntityAllocator.onFindEntityByRguidFn(guid);
}


ResultCode GameEntity::serialize(Archive* pArchive) const
{
    // TODO: Need to figure out how to obtain a proper rguid.
    //       Parent needs to also be included as well!
    RGUID guid = getUUID();
    pArchive->write((void*)&guid, sizeof(RGUID));

    const std::string& s    = getName();
    U64 szBytes             = s.size();
    const char* str         = s.c_str();

    pArchive->write((void*)str, szBytes);

    const std::string tag   = getTag();
    szBytes                 = tag.size();
    str                     = tag.c_str();

    pArchive->write((void*)str, szBytes);

    return RecluseResult_Ok;
}

ResultCode GameEntity::deserialize(Archive* pArchive)
{
    return RecluseResult_NoImpl;
}
} // ECS
} // Recluse 