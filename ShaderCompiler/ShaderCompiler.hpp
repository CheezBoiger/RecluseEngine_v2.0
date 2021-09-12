//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include <vector>

namespace Recluse {


ErrType addShaderToCompile(const std::string& filePath, const std::string& config, ShaderType shaderType);

ErrType compileShaders(ShaderLang lang);

ErrType setConfigs(const std::string& configPath, U32 compilerIndex);
ErrType setShaderFiles(const std::string& shadersPath);
} // Recluse