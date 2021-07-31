//
#include "PreZRenderModule.hpp"

#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Renderer/RendererResources.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace PreZ {

GraphicsResourceView* pSceneDepthView = nullptr;

GraphicsPipeline* pOpaqueStaticMeshPipeline = nullptr;
GraphicsPipeline* pOpaqueSkinnedMeshPipeline = nullptr;
GraphicsPipeline* pOpaqueMorphTargetMeshPipeline = nullptr;

RenderPass* pPreZPass = nullptr;

void initialize(GraphicsDevice* pDevice, Engine::SceneBuffers* pBuffers)
{
    R_ASSERT(pBuffers->pSceneDepth != NULL);
    R_ASSERT(pBuffers->pSceneDepth->getResource() != NULL);
    R_ASSERT(pBuffers->pSceneDepth->getView() != NULL);

    ErrType result                              = REC_RESULT_OK;
    GraphicsPipelineStateDesc pipelineCi        = { };
    RenderPassDesc rpCi                         = { };
    const GraphicsResourceDescription& resDesc  = pBuffers->pSceneDepth->getResource()->getDesc();
    
    rpCi.width                  = resDesc.width;
    rpCi.height                 = resDesc.height;
    rpCi.numRenderTargets       = 0;
    rpCi.pDepthStencil          = pBuffers->pSceneDepth->getView();

    result = pDevice->createRenderPass(&pPreZPass, rpCi);

    if (result != REC_RESULT_OK) {
    
        R_ERR("PreZRenderModule", "Failed to create render pass!");

        return;
    
    }

    pipelineCi.raster.cullMode = CULL_MODE_BACK;
    pipelineCi.raster.frontFace = FRONT_FACE_CLOCKWISE;
    pipelineCi.raster.lineWidth = 1.0f;
    pipelineCi.raster.polygonMode = POLYGON_MODE_FILL;
    pipelineCi.raster.depthBiasEnable = true;

    pipelineCi.pVS = nullptr;

    pipelineCi.ds.depthTestEnable = true;
    pipelineCi.ds.depthWriteEnable = true;
    pipelineCi.ds.minDepthBounds = 1.0f; // Reverse for preZ
    pipelineCi.ds.maxDepthBounds = 0.0f;
    pipelineCi.ds.stencilTestEnable = false;
    pipelineCi.ds.depthCompareOp = COMPARE_OP_GREATER;

    pipelineCi.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


    
    
}


void destroy(GraphicsDevice* pDevice)
{
}


void generate(GraphicsCommandList* pCommandList)
{
}
} // PreZ
} // Recluse