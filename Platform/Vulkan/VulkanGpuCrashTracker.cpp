// 
#include "VulkanGpuCrashTracker.hpp"

#include <Recluse/Messaging.hpp>
#include <Recluse/System/Architecture.hpp>

#ifdef RCL_ENABLE_AFTERMATH
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>
#endif

#include <Recluse/Filesystem/Filesystem.hpp>

#include <fstream>

#include <chrono>

namespace Recluse {
namespace Vulkan {

#ifdef RCL_ENABLE_AFTERMATH
R_DECLARE_GLOBAL_STRING(g_fileName, "GpuCrashVulkan", "GpuCrash.Filename");


ResultCode AftermathGpuCrashTracker::initialize()
{
	bool succeeded = GFSDK_Aftermath_EnableGpuCrashDumps(
		GFSDK_Aftermath_Version_API,
		GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
		GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks,
		GpuCrashDumpCallback,
		ShaderDebugInfoCallback,
		CrashDumpDescriptionCallback,
		ResolveMarkerCallback,
		this);

	if (!GFSDK_Aftermath_SUCCEED(succeeded))
	{
		return RecluseResult_Failed;
	}

	m_initialized = true;
	R_INFO("GpuCrashTracker", "Nvidia Aftermath enabled for crash tracking.");

	return RecluseResult_Ok;
}

AftermathGpuCrashTracker::~AftermathGpuCrashTracker()
{
	if (isInitialized())
	{
		GFSDK_Aftermath_DisableGpuCrashDumps();
	}
}

ResultCode AftermathGpuCrashTracker::processCrash()
{
	if (!isInitialized())
		return RecluseResult_InvalidVersion;

	GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
	if (!GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GetCrashDumpStatus(&status)))
	{
		return RecluseResult_Failed;
	}

	// Need to give the tracker time to process.
	R_ERROR("GpuCrashTracker", "Writing gpu crash dump to file.");
	
	while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
			status != GFSDK_Aftermath_CrashDump_Status_Finished)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (!GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GetCrashDumpStatus(&status)))
		{
			return RecluseResult_Failed;
		}
	}

	if (status != GFSDK_Aftermath_CrashDump_Status_Finished)
	{
		R_ERROR("GpuCrashTracker", "Unexpected error when attempting gpu crash minidump=%d", status);
		return RecluseResult_Failed;
	}

	R_ERROR("GpuCrashTracker", "Finished crash dump.");
	return RecluseResult_Ok;
}


ResultCode AftermathGpuCrashTracker::writeCrashDumpToFile(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
{
	GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};
	bool succeeded = GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GpuCrashDump_CreateDecoder(
		GFSDK_Aftermath_Version_API,
		pGpuCrashDump,
		gpuCrashDumpSize,
		&decoder));

	if (!succeeded)
		return RecluseResult_Failed;

	// Use the decoder object to read basic information, like application
// name, PID, etc. from the GPU crash dump.
	GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo = {};
	succeeded = GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo));

	// Use the decoder object to query the application name that was set
	// in the GPU crash dump description.
	uint32_t applicationNameLength = 0;
	succeeded = GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize(
		decoder,
		GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
		&applicationNameLength));

	std::vector<char> applicationName(applicationNameLength, '\0');

	succeeded = GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GpuCrashDump_GetDescription(
		decoder,
		GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
		uint32_t(applicationName.size()),
		applicationName.data()));

	// Create a unique file name for writing the crash dump data to a file.
	// Note: due to an Nsight Aftermath bug (will be fixed in an upcoming
	// driver release) we may see redundant crash dumps. As a workaround,
	// attach a unique count to each generated file name.
	static int count = 0;
	const std::string baseFileName =
		std::string(applicationName.data())
		+ "-"
		+ std::to_string(baseInfo.pid)
		+ "-"
		+ std::to_string(++count);

	std::string crashFileName = baseFileName + ".nv-gpudmp";
	std::ofstream dumpFile(crashFileName, std::ios::out | std::ios::binary);

	if (dumpFile)
	{
		dumpFile.write((const char*)pGpuCrashDump, gpuCrashDumpSize);
		dumpFile.close();	
	}

	succeeded = GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GpuCrashDump_DestroyDecoder(decoder));

	return succeeded ? RecluseResult_Ok : RecluseResult_Failed;
}


ResultCode AftermathGpuCrashTracker::onCrashDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription)
{
	// Add some basic description about the crash. This is called after the GPU crash happens, but before
	// the actual GPU crash dump callback. The provided data is included in the crash dump and can be
	// retrieved using GFSDK_Aftermath_GpuCrashDump_GetDescription().
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, g_fileName.c_str());
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationVersion, "v1.0");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined, "This is a GPU crash dump, pretty fancy.");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 1, "Engine State: Rendering.");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 2, "More user-defined information...");

	return RecluseResult_Ok;
}


ResultCode AftermathGpuCrashTracker::onCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
{
	std::lock_guard<std::mutex> _lck(m_mutex);

	return writeCrashDumpToFile(pGpuCrashDump, gpuCrashDumpSize);
}

void AftermathGpuCrashTracker::GpuCrashDumpCallback(
	const void* pGpuCrashDump,
	const uint32_t gpuCrashDumpSize,
	void* pUserData)
{
	AftermathGpuCrashTracker* pGpuCrashTracker = reinterpret_cast<AftermathGpuCrashTracker*>(pUserData);
	pGpuCrashTracker->onCrashDump(pGpuCrashDump, gpuCrashDumpSize);
}

// Shader debug information callback.
void AftermathGpuCrashTracker::ShaderDebugInfoCallback(
	const void* pShaderDebugInfo,
	const uint32_t shaderDebugInfoSize,
	void* pUserData)
{

}

// GPU crash dump description callback.
void AftermathGpuCrashTracker::CrashDumpDescriptionCallback(
	PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription,
	void* pUserData)
{
	AftermathGpuCrashTracker* pGpuCrashTracker = reinterpret_cast<AftermathGpuCrashTracker*>(pUserData);
	pGpuCrashTracker->onCrashDescription(addDescription);
}

// App-managed marker resolve callback
void AftermathGpuCrashTracker::ResolveMarkerCallback(
	const void* pMarkerData,
	const uint32_t markerDataSize,
	void* pUserData,
	PFN_GFSDK_Aftermath_ResolveMarker resolveMarker)
{

}

// Shader debug information lookup callback.
void AftermathGpuCrashTracker::ShaderDebugInfoLookupCallback(
	const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier,
	PFN_GFSDK_Aftermath_SetData setShaderDebugInfo,
	void* pUserData)
{

}

// Shader lookup callback.
void AftermathGpuCrashTracker::ShaderLookupCallback(
	const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash,
	PFN_GFSDK_Aftermath_SetData setShaderBinary,
	void* pUserData)
{

}

// Shader source debug info lookup callback.
void AftermathGpuCrashTracker::ShaderSourceDebugInfoLookupCallback(
	const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName,
	PFN_GFSDK_Aftermath_SetData setShaderBinary,
	void* pUserData)
{

}
#endif
} // Vulkan
} // Recluse