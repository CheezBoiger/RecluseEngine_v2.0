//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Renderer/RenderCommand.hpp"

namespace Recluse {
namespace AOV {


void generate(GraphicsContext* context, Engine::CommandList* pHiCmdList, Engine::KeyId* keys, U64 sz);

} // AOV
} // Recluse