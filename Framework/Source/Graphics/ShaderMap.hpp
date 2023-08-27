//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/ShaderBuilder.hpp"
#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {

class Shader;

typedef ShaderBuilder*(*ShaderBuilderFunc)(ShaderIntermediateCode);

ShaderBuilder* createGlslangShaderBuilder(ShaderIntermediateCode imm);
ShaderBuilder* createDxcShaderBuilder(ShaderIntermediateCode imm);

namespace ShaderMap {


typedef U32 ShaderPermutation;

B32 storeShader(Hash64 nameHash, Shader* pShader, ShaderPermutation permutation);
Shader* getShader(Hash64 nameHash, ShaderPermutation permutation);
} // ShaderMap
} // Recluse