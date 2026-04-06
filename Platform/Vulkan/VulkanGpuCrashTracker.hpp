//
#pragma once

#include "VulkanCommons.hpp"
#include "VulkanResource.hpp"

#ifdef RCL_ENABLE_AFTERMATH
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>
#endif

#include <map>
#include <string>
#include <vector>
#include <mutex>

namespace Recluse {
namespace Vulkan {


class GpuCrashShaderDatabase
{
public:

private:
};

class GpuCrashTracker
{
public:
    virtual ~GpuCrashTracker() {}

    virtual ResultCode initialize(const std::string& applicationName) = 0;
    virtual ResultCode processCrash() = 0;
};

#ifdef RCL_ENABLE_AFTERMATH
class AftermathGpuCrashTracker : public GpuCrashTracker
{
public:
    virtual             ~AftermathGpuCrashTracker();

	virtual ResultCode initialize(const std::string& applicationName) override;
    
	// Process the crash, and report it to a file for external debugging.
	// Call this function right after a device lost.
	virtual ResultCode  processCrash() override;

    Bool                isInitialized() const { return m_initialized; }

    ResultCode          onCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);
    ResultCode          onCrashDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription);

    ResultCode          writeCrashDumpToFile(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);

private:
    Bool m_initialized = false;
    std::string m_applicationName;

    mutable std::mutex m_mutex;

    // GPU crash dump callback.
    static void GpuCrashDumpCallback(
        const void* pGpuCrashDump,
        const uint32_t gpuCrashDumpSize,
        void* pUserData);

    // Shader debug information callback.
    static void ShaderDebugInfoCallback(
        const void* pShaderDebugInfo,
        const uint32_t shaderDebugInfoSize,
        void* pUserData);

    // GPU crash dump description callback.
    static void CrashDumpDescriptionCallback(
        PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription,
        void* pUserData);

    // App-managed marker resolve callback
    static void ResolveMarkerCallback(
        const void* pMarkerData,
        const uint32_t markerDataSize,
        void* pUserData,
        PFN_GFSDK_Aftermath_ResolveMarker resolveMarker);

    // Shader debug information lookup callback.
    static void ShaderDebugInfoLookupCallback(
        const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier,
        PFN_GFSDK_Aftermath_SetData setShaderDebugInfo,
        void* pUserData);

    // Shader lookup callback.
    static void ShaderLookupCallback(
        const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash,
        PFN_GFSDK_Aftermath_SetData setShaderBinary,
        void* pUserData);

    // Shader source debug info lookup callback.
    static void ShaderSourceDebugInfoLookupCallback(
        const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName,
        PFN_GFSDK_Aftermath_SetData setShaderBinary,
        void* pUserData);
};
#endif
} // Vulkan
} // Recluse