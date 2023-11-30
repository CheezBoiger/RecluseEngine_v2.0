//
#pragma once
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"

namespace Recluse {
namespace Pipeline {

typedef ShaderBuilder*(*ShaderBuilderFunc)(ShaderIntermediateCode);

ShaderBuilder* createGlslangShaderBuilder(ShaderIntermediateCode imm);
ShaderBuilder* createDxcShaderBuilder(ShaderIntermediateCode imm);
} // Pipeline
} // Recluse