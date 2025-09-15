//
#include "Recluse/Utility.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Serialization/Hasher.hpp"

#include <unordered_map>
#include <memory>

namespace Recluse {
namespace GlobalCommands {
namespace Internal {
static CriticalSectionGuard                 g_commandCs = { };
static std::unique_ptr<std::map<Hash64, DataListener*>> g_commandMap;
static Bool g_isInitialized = false;


R_INTERNAL
void checkInitializeCommandDatabase()
{
    if (!g_commandCs.isInitialized())
    {
        g_commandCs.initialize();
    }

    ScopedCriticalSection _(g_commandCs);

    if (!g_isInitialized)
    {
        g_commandMap = std::make_unique<std::map<Hash64, DataListener*>>();
        g_isInitialized = true;
    }
}

DataListener::DataListener(const std::string& command, void* globalVariable)
    : value(globalVariable)
{
    // For now, we need to lazy allocate this.
    storeData(command, this);
}


void DataListener::storeData(const std::string& command, DataListener* data)
{
    checkInitializeCommandDatabase();

    Hash64 h = recluseHashFast(command.data(), command.size() * sizeof(char));
    //g_commandMap.insert(std::make_pair(h, data)); // try g_commandMap[h] = data instead.
    g_commandMap->insert(std::make_pair(h, data));
}


DataListener* obtainData(const std::string& command)
{
    if (!g_isInitialized)
        return nullptr;

    Hash64 h = recluseHashFast(command.data(), command.size() * sizeof(char));
    auto iter = Internal::g_commandMap->find(h);
    if (iter != Internal::g_commandMap->end())
    {
        return iter->second;
    }
    return nullptr;
}


Bool setData(const std::string& command, const void* value, size_t sizeBytesToWrite)
{
    checkInitializeCommandDatabase();

    Hash64 h = recluseHashFast(command.data(), command.size() * sizeof(char));

    auto iter = Internal::g_commandMap->find(h);
    if (iter != Internal::g_commandMap->end())
    {
        const size_t sizeToWrite = sizeBytesToWrite;
        memcpy(iter->second->value, value, sizeToWrite);
        return true;
    }
    return false;
}


Bool setDataAsString(const std::string& command, const char* value)
{
    checkInitializeCommandDatabase();

    Hash64 h = recluseHashFast(command.data(), command.size() * sizeof(char));

    auto iter = Internal::g_commandMap->find(h);
    if (iter != Internal::g_commandMap->end())
    {
        std::string* pData = reinterpret_cast<std::string*>(iter->second->value);
        *pData = value;
        return true;
    }
    return false;
}
} // Internal
} // GlobalCommands
} // Recluse