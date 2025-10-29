//

#include "Recluse/Renderer/RenderCommand.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {
namespace LightCluster {


void cullLights(GraphicsContext* context);
void combineForward(GraphicsContext* context, Engine::KeyId* keys, U64 sz);
void combineDeferred(GraphicsContext* context);
} // LightCluster
} // Recluse