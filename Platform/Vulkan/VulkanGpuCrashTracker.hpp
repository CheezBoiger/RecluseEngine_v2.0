//
#pragma once

#include "VulkanCommons.hpp"
#include "VulkanResource.hpp"

#ifdef RCL_ENABLE_AFTERMATH
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>
#endif

#include <iomanip>
#include <ostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <mutex>

namespace std
{
template<typename T>
inline std::string to_hex_string(T n)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2 * sizeof(T)) << std::hex << n;
    return stream.str();
}

inline std::string to_string(GFSDK_Aftermath_Result result)
{
    return std::string("0x") + to_hex_string(static_cast<uint32_t>(result));
}

inline std::string to_string(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier)
{
    return to_hex_string(identifier.id[0]) + "-" + to_hex_string(identifier.id[1]);
}

inline std::string to_string(const GFSDK_Aftermath_ShaderBinaryHash& hash)
{
    return to_hex_string(hash.hash);
}
} // namespace std

//*********************************************************
// Helper for comparing shader hashes and debug info identifier.
//

// Helper for comparing GFSDK_Aftermath_ShaderDebugInfoIdentifier.
inline bool operator<(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& lhs, const GFSDK_Aftermath_ShaderDebugInfoIdentifier& rhs)
{
    if (lhs.id[0] == rhs.id[0])
    {
        return lhs.id[1] < rhs.id[1];
    }
    return lhs.id[0] < rhs.id[0];
}

// Helper for comparing GFSDK_Aftermath_ShaderBinaryHash.
inline bool operator<(const GFSDK_Aftermath_ShaderBinaryHash& lhs, const GFSDK_Aftermath_ShaderBinaryHash& rhs)
{
    return lhs.hash < rhs.hash;
}

// Helper for comparing GFSDK_Aftermath_ShaderDebugName.
inline bool operator<(const GFSDK_Aftermath_ShaderDebugName& lhs, const GFSDK_Aftermath_ShaderDebugName& rhs)
{
    return strncmp(lhs.name, rhs.name, sizeof(lhs.name)) < 0;
}

namespace Recluse {
namespace Vulkan {


class GpuCrashShaderDatabase
{
public:

    Bool registerShader(const uint8_t* byteCode, U64 sizeBytes);
    Bool lookup(const GFSDK_Aftermath_ShaderBinaryHash& shaderKey, std::vector<uint8_t>& data) const;

private:
    std::map<uint64_t, std::vector<uint8_t>>
        m_shaderMap;
};

class GpuCrashTracker
{
public:
    virtual ~GpuCrashTracker() {}

    virtual ResultCode initialize(const std::string& applicationName) = 0;
    virtual ResultCode processCrash() = 0;

    virtual GpuCrashShaderDatabase* getShaderDatabase() = 0;
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
    ResultCode          onShaderLookup(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary) const;
    ResultCode          onShaderDebugInfo(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize);
    ResultCode          onShaderDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo) const;

    ResultCode          writeCrashDumpToFile(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);
    // Helper for writing shader debug information to a file
    ResultCode          writeShaderDebugInformationToFile(
        GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier,
        const void* pShaderDebugInfo,
        const uint32_t shaderDebugInfoSize);

    virtual GpuCrashShaderDatabase* getShaderDatabase() { return &m_shaderDatabase; }
private:
    Bool m_initialized = false;
    std::string m_applicationName;

    std::map<GFSDK_Aftermath_ShaderDebugInfoIdentifier, std::vector<uint8_t>> m_shaderDebugInfo;

    GpuCrashShaderDatabase m_shaderDatabase;

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