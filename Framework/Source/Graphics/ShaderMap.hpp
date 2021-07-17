//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {
namespace ShaderMap {



B32 storeShader(Hash128 hash, Shader* pShader);
} // ShaderMap
} // Recluse