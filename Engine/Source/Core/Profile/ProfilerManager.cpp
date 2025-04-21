//
#include "Recluse/Core/Profile/Profiler.hpp"

#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Threading/Threading.hpp"

#include <unordered_map>
#if defined(RECLUSE_DEBUG) || defined(RECLUSE_DEVELOPER)

namespace Recluse {
namespace CpuProfileDatabase {

MutexGuard CpuDatabaseMutex = MutexGuard("ProfilerCpuMutex");

using CpuPerfInfo = CpuPerformanceProfile::PerformanceMeasurement;

std::unordered_map<Hash64, CpuPerfInfo> measures;

ResultCode storeMeasurement(const std::string& profileName, const CpuPerformanceProfile::PerformanceMeasurement& measurement, const std::string& groupName)
{
    std::string name = groupName + profileName;
    Hash64 h = recluseHashFast(name.c_str(), name.size());

    ScopedLock _lck(CpuDatabaseMutex);

    measures[h] = measurement;    

    return RecluseResult_Ok;
}


ResultCode queryMeasurement(const std::string& profileName, CpuPerformanceProfile::PerformanceMeasurement& output, const std::string& groupName)
{
    std::string name = groupName + profileName;
    Hash64 h = recluseHashFast(name.c_str(), name.size());
    ResultCode result = RecluseResult_Failed;

    ScopedLock _lck(CpuDatabaseMutex);
    
    auto it = measures.find(h);
    if (it != measures.end())
    {
        output = it->second;
        result = RecluseResult_Ok;
    }
    return result;    
}
} // CpuProfileDatabase
} // Recluse
#endif