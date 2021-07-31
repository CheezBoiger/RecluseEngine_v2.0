//
#include "PreZRenderModule.hpp"

#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

namespace Recluse {
namespace PreZ {

GraphicsResourceView* pSceneDepthView = nullptr;

GraphicsPipeline* pOpaqueStaticMeshPipeline = nullptr;
GraphicsPipeline* pOpaqueDynamicMeshPipeline = nullptr;
GraphicsPipeline* pOpaqueMorphTargetMeshPipeline = nullptr;

void initialize(GraphicsDevice* pDevice, Engine::SceneBuffers* pBuffers)
{
}


void destroy(GraphicsDevice* pDevice)
{
}


void generate(GraphicsCommandList* pCommandList)
{
}
} // PreZ
} // Recluse