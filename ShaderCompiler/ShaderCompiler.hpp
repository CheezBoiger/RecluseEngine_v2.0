//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include <vector>

namespace Recluse {

///////////////////////////////////////////////////////////////////////////////////////////
// 
// Shader Compiler API
// 
///////////////////////////////////////////////////////////////////////////////////////////
R_PUBLIC_API ErrType				addShaderToCompile(const std::string& filePath, const char* entryPoint, const std::string& config, ShaderType shaderType);
R_PUBLIC_API ErrType				compileShaders(ShaderLang lang);
R_PUBLIC_API ErrType				setConfigs(const std::string& configPath, U32 compilerIndex);
R_PUBLIC_API ErrType				setShaderFiles(const std::string& shadersPath);
extern ShaderLang	getDefaultShaderLanguage();
} // Recluse