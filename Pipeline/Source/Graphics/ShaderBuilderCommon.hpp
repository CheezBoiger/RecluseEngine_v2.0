//
#pragma once
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"

namespace Recluse {
namespace Pipeline {

typedef ShaderBuilder*(*ShaderBuilderFunc)();

ShaderBuilder* createGlslangShaderBuilder();
ShaderBuilder* createDxcShaderBuilder();
} // Pipeline
} // Recluse