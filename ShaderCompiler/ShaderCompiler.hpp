//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include <vector>

namespace Recluse {


ErrType addShaderToCompile(const std::string& filePath, const std::string& config, ShaderType shaderType);

ErrType compileShaders(const std::string& sourcePath, ShaderLang lang);

ErrType setConfigs(const std::string& configPath);
} // Recluse