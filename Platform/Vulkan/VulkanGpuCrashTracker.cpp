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
#include <string>

#include <chrono>

namespace Recluse {
namespace Vulkan {

#ifdef RCL_ENABLE_AFTERMATH
R_DECLARE_GLOBAL_STRING(g_fileName, "GpuCrashVulkan", "GpuCrash.Filename");


Bool AftermathGpuCrashShaderDatabase::registerShader(const uint8_t* bytecode, U64 sizeBytes)
{
	if (sizeBytes == 0 || !bytecode)
		return false;

	const GFSDK_Aftermath_SpirvCode spirvCode = { bytecode, sizeBytes };
	GFSDK_Aftermath_ShaderBinaryHash shaderHash;

	GFSDK_Aftermath_Result result = GFSDK_Aftermath_GetShaderHashSpirv(
		GFSDK_Aftermath_Version_API, &spirvCode, &shaderHash);

	if (!GFSDK_Aftermath_SUCCEED(result))
	{
		return false;
	}

	auto it = m_shaderMap.find(shaderHash.hash);
	if (it == m_shaderMap.end())
	{
		std::vector<uint8_t> bytecodeData(sizeBytes);
		memcpy(bytecodeData.data(), bytecode, sizeBytes);
		m_shaderMap.insert(std::make_pair(shaderHash.hash, bytecodeData));
	}
	else
	{
		return false;
	}

	return true;
}


Bool AftermathGpuCrashShaderDatabase::lookup(const GFSDK_Aftermath_ShaderBinaryHash& shaderKey, std::vector<uint8_t>& data) const
{
	auto it = m_shaderMap.find(shaderKey.hash);
	if (it == m_shaderMap.end())
	{
		return false;
	}
	data = it->second;
	return true;
}


ResultCode AftermathGpuCrashTracker::initialize(const std::string& applicationName)
{
	GFSDK_Aftermath_Result succeeded = GFSDK_Aftermath_EnableGpuCrashDumps(
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

	m_applicationName = applicationName;
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


ResultCode AftermathGpuCrashTracker::onShaderDebugInfo(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize)
{
	// Make sure only one thread at a time...
	std::lock_guard<std::mutex> lock(m_mutex);

	// Get shader debug information identifier
	GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier = {};
	GFSDK_Aftermath_GetShaderDebugInfoIdentifier(
		GFSDK_Aftermath_Version_API,
		pShaderDebugInfo,
		shaderDebugInfoSize,
		&identifier);

	// Store information for decoding of GPU crash dumps with shader address mapping
	// from within the application.
	std::vector<uint8_t> data((uint8_t*)pShaderDebugInfo, (uint8_t*)pShaderDebugInfo + shaderDebugInfoSize);
	m_shaderDebugInfo[identifier].swap(data);

	// Write to file for later in-depth analysis of crash dumps with Nsight Graphics
	writeShaderDebugInformationToFile(identifier, pShaderDebugInfo, shaderDebugInfoSize);
	return RecluseResult_Ok;
}


ResultCode  AftermathGpuCrashTracker::writeShaderDebugInformationToFile(
	GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier,
	const void* pShaderDebugInfo,
	const uint32_t shaderDebugInfoSize)
{
	// Create a unique file name.
	const std::string filePath = "shader-" + std::to_string(identifier) + ".nvdbg";

	std::ofstream f(filePath, std::ios::out | std::ios::binary);
	if (f)
	{
		f.write((const char*)pShaderDebugInfo, shaderDebugInfoSize);
	}
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

	// Decode the crash dump to a JSON string.
	// Step 1: Generate the JSON and get the size.
	uint32_t jsonSize = 0;
	succeeded = GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GpuCrashDump_GenerateJSON(
		decoder,
		GFSDK_Aftermath_GpuCrashDumpDecoderFlags_ALL_INFO,
		GFSDK_Aftermath_GpuCrashDumpFormatterFlags_NONE,
		ShaderDebugInfoLookupCallback,
		ShaderLookupCallback,
		ShaderSourceDebugInfoLookupCallback,
		this,
		&jsonSize));
	// Step 2: Allocate a buffer and fetch the generated JSON.
	std::vector<char> json(jsonSize);
	succeeded = GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GpuCrashDump_GetJSON(
		decoder,
		uint32_t(json.size()),
		json.data()));

	// Write the crash dump data as JSON to a file.
	const std::string jsonFileName = crashFileName + ".json";
	std::ofstream jsonFile(jsonFileName, std::ios::out | std::ios::binary);
	if (jsonFile)
	{
		// Write the JSON to the file (excluding string termination)
		jsonFile.write(json.data(), json.size() - 1);
		jsonFile.close();
	}

	succeeded = GFSDK_Aftermath_SUCCEED(GFSDK_Aftermath_GpuCrashDump_DestroyDecoder(decoder));

	return succeeded ? RecluseResult_Ok : RecluseResult_Failed;
}


ResultCode AftermathGpuCrashTracker::onCrashDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription)
{
	// Add some basic description about the crash. This is called after the GPU crash happens, but before
	// the actual GPU crash dump callback. The provided data is included in the crash dump and can be
	// retrieved using GFSDK_Aftermath_GpuCrashDump_GetDescription().
	std::string& appName = g_fileName;

	if (!m_applicationName.empty())
		appName = m_applicationName;

	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, appName.c_str());
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationVersion, "v1.0");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined, "This is a GPU crash dump, pretty fancy.");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 1, "Engine State: Rendering.");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 2, "More user-defined information...");

	return RecluseResult_Ok;
}

ResultCode AftermathGpuCrashTracker::onShaderDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo) const
{
	// Search the list of shader debug information blobs received earlier.
	auto i_debugInfo = m_shaderDebugInfo.find(identifier);
	if (i_debugInfo == m_shaderDebugInfo.end())
	{
		// Early exit, nothing found. No need to call setShaderDebugInfo.
		return RecluseResult_NotFound;
	}

	// Let the GPU crash dump decoder know about the shader debug information
	// that was found.
	setShaderDebugInfo(i_debugInfo->second.data(), uint32_t(i_debugInfo->second.size()));
	return RecluseResult_Ok;
}

ResultCode AftermathGpuCrashTracker::onShaderLookup(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary) const
{
	std::vector<uint8_t> data;

	if (!m_shaderDatabase.lookup(shaderHash, data))
	{
		return RecluseResult_Failed;
	}

	setShaderBinary(data.data(), uint32_t(data.size()));
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
	AftermathGpuCrashTracker* pGpuCrashTracker = reinterpret_cast<AftermathGpuCrashTracker*>(pUserData);
	pGpuCrashTracker->onShaderDebugInfo(pShaderDebugInfo, shaderDebugInfoSize);
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
	AftermathGpuCrashTracker* pGpuCrashTracker = reinterpret_cast<AftermathGpuCrashTracker*>(pUserData);
	pGpuCrashTracker->onShaderDebugInfoLookup(*pIdentifier, setShaderDebugInfo);
}

// Shader lookup callback.
void AftermathGpuCrashTracker::ShaderLookupCallback(
	const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash,
	PFN_GFSDK_Aftermath_SetData setShaderBinary,
	void* pUserData)
{
	AftermathGpuCrashTracker* pGpuCrashTracker = reinterpret_cast<AftermathGpuCrashTracker*>(pUserData);
	pGpuCrashTracker->onShaderLookup(*pShaderHash, setShaderBinary);
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