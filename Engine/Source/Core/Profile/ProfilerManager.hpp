
#pragma once

#include "Recluse/Core/Profile/Profiler.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"

namespace Recluse {
namespace CpuProfileDatabase {


ResultCode storeMeasurement(const std::string& profileName, const CpuPerformanceProfile::PerformanceMeasurement& measurement, const std::string& groupName = std::string());
ResultCode queryMeasurement(const std::string& profileName, CpuPerformanceProfile::PerformanceMeasurement& output, const std::string& groupName = std::string());
} // CpuProfileDatabase
} // Recluse