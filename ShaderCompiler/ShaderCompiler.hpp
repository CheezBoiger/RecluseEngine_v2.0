//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include <vector>

namespace Recluse {


ErrType addShaderToCompile();

ErrType compileShaders(ShaderLang lang);
} // Recluse