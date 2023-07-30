//
#include "Recluse/Utility.hpp"
#include "Recluse/Threading/Threading.hpp"


namespace Recluse {
namespace GlobalCommands {
namespace Internal {
CriticalSectionGuard              g_commandCs = { };
std::map<std::string, DataListener*> g_commandMap;

DataListener::DataListener(const std::string& command, void* globalVariable)
    : value(globalVariable)         
{
    storeData(command, this);
}


void DataListener::storeData(const std::string& command, DataListener* data)
{
    if (!g_commandCs.isInitialized())
    {
        g_commandCs.initialize();
    }
    ScopedCriticalSection _(g_commandCs);
    g_commandMap.insert(std::make_pair(command, data));
}


DataListener* obtainData(const std::string& command)
{
    auto iter = Internal::g_commandMap.find(command);
    if (iter != Internal::g_commandMap.end())
    {
        return iter->second;
    }
    return nullptr;
}


Bool setData(const std::string& command, const void* value, size_t sizeBytesToWrite)
{
    ScopedCriticalSection _(g_commandCs);
    auto iter = Internal::g_commandMap.find(command);
    if (iter != Internal::g_commandMap.end())
    {
        const size_t sizeToWrite = sizeBytesToWrite;
        memcpy(iter->second->value, value, sizeToWrite);
        return true;
    }
    return false;
}


Bool setDataAsString(const std::string& command, const char* value)
{
    ScopedCriticalSection _(g_commandCs);
    auto iter = Internal::g_commandMap.find(command);
    if (iter != Internal::g_commandMap.end())
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