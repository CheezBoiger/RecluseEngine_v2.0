//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {

class Shader;

namespace ShaderMap {


typedef U32 ShaderPermutation;

B32 storeShader(Hash64 nameHash, Shader* pShader, ShaderPermutation permutation);
Shader* getShader(Hash64 nameHash, ShaderPermutation permutation);
} // ShaderMap
} // Recluse