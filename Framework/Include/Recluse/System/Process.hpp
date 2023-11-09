//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {
namespace Process {


enum Architecture
{
    // Used by intel 32-bit architectures.
    Architecture_x86,
    // Used by both intel and amd 64-bit architectures.
    Architecture_x64,
    // Arm 32-bit architecture.
    Architecture_Arm32,
    // Arm 64-bit architecture.
    Archictecture_Amd64
};

struct CpuInfo
{
    U32             numberCoreProcessors;
    U32             numberLogicalProcessors;
    U32             numberLogicalProcessorsPerCore;
    Architecture    processorArchitecture;
};


R_OS_CALL ResultCode queryCpuInfo(CpuInfo& cpuInfo);
} // Process
} // Recluse