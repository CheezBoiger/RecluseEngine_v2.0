//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/Math/Vector4.hpp"

#include "RecluseEngine_exports.hpp"

namespace Recluse {

#if defined(RECLUSE_DEBUG) || defined(RECLUSE_DEVELOPER)

// Cpu based profiler.
class RecluseEngine_PUBLIC_API CpuPerformanceProfile
{
public:
    CpuPerformanceProfile(const std::string& profilename, Math::Color4& color, const std::string& groupName = std::string());
    ~CpuPerformanceProfile();

    void stop();
    
    struct PerformanceMeasurement
    {
        Math::Color4 color;
        float milliseconds;
    };

    // Query a specific cpu measurement from the database.
    static PerformanceMeasurement query(const std::string& profileName, const std::string& groupName = std::string());
    
private:
    // Name of the profile to store the data in.
    std::string     profileName;
    std::string     groupName;
    Math::Color4    m_color;

    // Time start.
    RealtimeStopWatch start;
};


class GpuPerformanceProfile
{
public:
};


#define R_BEGIN_CPU_PROFILE(name, color, ...) \
    CpuPerformanceProfile _$Profiler##name(#name, color, #__VA_ARGS__) \
    _$Profiler.begin()
#define R_END_CPU_PROFILE() \
    _$Profiler.end()

// Runs a scoped cpu profile. Name is a string without quotations (will be made into a c string on compile.
// ... allows for adding a group name.
#define R_SCOPED_CPU_PROFILER(name, color, ...) CpuPerformanceProfile _$Profiler##name(#name, color, #__VA_ARGS__)
// Runs a scoped gpu profile. This macro requires being within the rendering commmand begin scope.
// 
#define R_SCOPED_GPU_PROFILER(name, color, ...)
#else
#define R_BEGIN_CPU_PROFILE(name, color, ...)
#define R_END_CPU_PROFILE()

#define R_SCOPED_CPU_PROFILER(name, color, ...)
#endif

} // Recluse
