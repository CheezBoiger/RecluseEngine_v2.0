//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {

class Shader;

namespace ShaderMap {



B32 storeShader(Hash64 hash, Shader* pShader);
} // ShaderMap
} // Recluse