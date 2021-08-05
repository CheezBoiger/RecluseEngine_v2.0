//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Renderer/RenderCommand.hpp"

namespace Recluse {
namespace AOV {


void generate(GraphicsCommandList* pCommandList, Engine::RenderCommandList* pHiCmdList, U64* keys, U64 sz);

} // AOV
} // Recluse