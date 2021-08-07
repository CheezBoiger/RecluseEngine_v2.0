//
#include "PreZRenderModule.hpp"

#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Renderer/RendererResources.hpp"

#include "Recluse/Filesystem/Filesystem.hpp"

#include "Recluse/Messaging.hpp"

#include <unordered_map>

namespace Recluse {
namespace PreZ {

GraphicsResourceView* pSceneDepthView = nullptr;

std::unordered_map<Engine::SubMeshRenderFlags, PipelineState*> pipelines;
RenderPass* pPreZPass = nullptr;

void initialize(GraphicsDevice* pDevice, Engine::SceneBuffers* pBuffers)
{
    R_ASSERT(pBuffers->pSceneDepth                  != NULL);
    R_ASSERT(pBuffers->pSceneDepth->getResource()   != NULL);
    R_ASSERT(pBuffers->pSceneDepth->getView()       != NULL);

    R_DEBUG("PreZ", "Initializing preZ render pass...");

    std::string staticVertShader = Engine::getBinaryShaderPath("Geometry/VertFactoryStatic");
    
    Shader* pShader = Shader::create(Engine::intermediateCode, SHADER_TYPE_VERTEX);
    FileBufferData data = { };

    File::readFrom(&data, staticVertShader);
    pShader->load(data.buffer.data(), data.buffer.size());

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

    pipelineCi.raster.cullMode          = CULL_MODE_BACK;
    pipelineCi.raster.frontFace         = FRONT_FACE_CLOCKWISE;
    pipelineCi.raster.lineWidth         = 1.0f;
    pipelineCi.raster.polygonMode       = POLYGON_MODE_FILL;
    pipelineCi.raster.depthBiasEnable   = true;

    pipelineCi.pVS                  = nullptr;

    pipelineCi.ds.depthTestEnable   = true;
    pipelineCi.ds.depthWriteEnable  = true;
    pipelineCi.ds.minDepthBounds    = 1.0f; // Reverse for preZ
    pipelineCi.ds.maxDepthBounds    = 0.0f;
    pipelineCi.ds.stencilTestEnable = false;
    pipelineCi.ds.depthCompareOp    = COMPARE_OP_GREATER;

    pipelineCi.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}


void destroy(GraphicsDevice* pDevice)
{
}


void generate(GraphicsCommandList* pCommandList, Engine::RenderCommandList* pMeshCommandList, U64* keys, U64 sz)
{
    Engine::RenderCommand** pRenderCommands      = pMeshCommandList->getRenderCommands();
    const GraphicsResourceDescription& depth    = pSceneDepthView->getResource()->getDesc();
    Rect depthRect      = { };
    depthRect.x = depthRect.y = 0.f;

    depthRect.width     = depth.width;
    depthRect.height    = depth.height;

    // Set the PreZ pass.
    pCommandList->setRenderPass(pPreZPass);
    pCommandList->clearDepthStencil(1.0f, 0, depthRect);

    for (U64 i = 0; i < sz; ++i) {
    
        U64 key = keys[i];
        Engine::RenderCommand* meshCmd              = pRenderCommands[key];
        Engine::SubMeshRenderFlags flags            = meshCmd->flags;
        U32 instanceCount                           = meshCmd->numInstances; 
        PipelineState* pipeline                 = pipelines[flags];

        pCommandList->bindVertexBuffers(meshCmd->numVertexBuffers, meshCmd->ppVertexBuffers, meshCmd->pOffsets);
        R_ASSERT(pipeline != NULL);

        pCommandList->setPipelineState(pipeline, BIND_TYPE_GRAPHICS);
        // TODO: Need to bind our descriptorSets when possible.
        pCommandList->bindDescriptorSets(0, nullptr, BIND_TYPE_GRAPHICS);

        switch (meshCmd->op) {

            case Engine::COMMAND_OP_DRAW_INSTANCED:
            {
                Engine::DrawRenderCommand* drawCmd = 
                    static_cast<Engine::DrawRenderCommand*>(meshCmd);
                pCommandList->drawInstanced(drawCmd->vertexCount,
                                            drawCmd->instanceCount,
                                            drawCmd->firstVertex,
                                            drawCmd->firstInstance);
            } break;
            case Engine::COMMAND_OP_DRAW_INDEXED_INSTANCED:        
            {
                Engine::DrawIndexedRenderCommand* indexedCmd = 
                    static_cast<Engine::DrawIndexedRenderCommand*>(meshCmd);
                pCommandList->drawIndexedInstanced(indexedCmd->indexCount, 
                                                   meshCmd->numInstances, 
                                                   indexedCmd->firstIndex, 
                                                   indexedCmd->vertexOffset, 
                                                   indexedCmd->firstInstance);
            } break;
        }
    
    }
}
} // PreZ
} // Recluse