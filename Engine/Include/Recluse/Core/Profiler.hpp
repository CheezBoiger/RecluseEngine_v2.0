//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector4.hpp"

namespace Recluse {

#if defined(RECLUSE_DEBUG) || defined(RECLUSE_DEVELOPER)

// Cpu based profiler.
class CpuPerformanceProfiler
{
public:
    CpuPerformanceProfiler(const std::string& profilename);
    ~CpuPerformanceProfiler();


    void end();
    void begin();
    
    Bool isRecording();
private:
    // Name of the profile to store the data in.
    std::string profileName;
};



#define R_BEGIN_CPU_PROFILE(name, color) \
    CpuPerformanceProfiler _$Profiler(name) \
    _$Profiler.begin()
#define R_END_CPU_PROFILE() \
    _$Profiler.end()

// Runs a scoped cpu profile.
#define R_SCOPED_CPU_PROFILER(name, color)
// Runs a scoped gpu profile.
#define R_SCOPED_GPU_PROFILER(name, color)
#else
#define R_BEGIN_CPU_PROFILE(name, color)
#define R_END_CPU_PROFILE()

#define R_SCOPED_CPU_PROFILER(name, color)
#endif

} // Recluse
