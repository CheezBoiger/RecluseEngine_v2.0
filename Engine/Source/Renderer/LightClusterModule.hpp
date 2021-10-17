//

#include "Recluse/Renderer/RenderCommand.hpp"
#include "Recluse/Graphics/CommandList.hpp"

namespace Recluse {
namespace LightCluster {


void cullLights(GraphicsCommandList* pCmdList);
void combineForward(GraphicsCommandList* pCmdList, U64* keys, U64 sz);
void combineDeferred(GraphicsCommandList* pCmdList);
} // LightCluster
} // Recluse