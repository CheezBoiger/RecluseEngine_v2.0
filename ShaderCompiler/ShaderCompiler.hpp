//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"

#include "RecluseSC_exports.hpp"

#include <vector>

namespace Recluse {

///////////////////////////////////////////////////////////////////////////////////////////
// 
// Shader Compiler API
// 
///////////////////////////////////////////////////////////////////////////////////////////
RecluseSC_PUBLIC_API ResultCode				addShaderToCompile(const std::string& filePath, const char* entryPoint, const std::string& config, ShaderType shaderType);
RecluseSC_PUBLIC_API ResultCode				compileShaders(ShaderLanguage lang);
RecluseSC_PUBLIC_API ResultCode				setConfigs(const std::string& configPath, U32 compilerIndex);
RecluseSC_PUBLIC_API ResultCode				setShaderFiles(const std::string& shadersPath);
extern ShaderLanguage	getDefaultShaderLanguage();
} // Recluse