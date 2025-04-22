//
#include "Recluse/Core/Profile/Profiler.hpp"
#include "Recluse/Time.hpp"

#include "ProfilerManager.hpp"

namespace Recluse {

CpuPerformanceProfile::CpuPerformanceProfile(const std::string& profileName, Math::Color4& color, const std::string& groupName)
    : groupName(groupName)
    , profileName(profileName)
    , m_color(color)
{
}


CpuPerformanceProfile::~CpuPerformanceProfile()
{
    stop();
}


void CpuPerformanceProfile::stop()
{
#if defined(RECLUSE_DEBUG) || defined(RECLUSE_DEVELOPER)
    RealtimeStopWatch end;
    RealtimeStopWatch totalTime = end - start;
    RealtimeTick tick = (RealtimeTick)totalTime;

    PerformanceMeasurement measurement = { };
    measurement.color = m_color;
    measurement.milliseconds = tick.delta() * 1000.0f; // need to store milliseconds.
    CpuProfileDatabase::storeMeasurement(profileName, measurement, groupName);
#endif
}


CpuPerformanceProfile::PerformanceMeasurement CpuPerformanceProfile::query(const std::string& profileName, const std::string& groupName)
{
    PerformanceMeasurement measurement;
#if defined(RECLUSE_DEBUG) || defined(RECLUSE_DEVELOPER)
    ResultCode result = CpuProfileDatabase::queryMeasurement(profileName, measurement, groupName);
    if (result != RecluseResult_Ok)
        measurement = { };
#else
    measurement = { };
#endif
    return measurement;
}
} // Recluse