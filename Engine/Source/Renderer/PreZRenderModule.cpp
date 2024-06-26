//
#include "PreZRenderModule.hpp"

#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Renderer/RendererResources.hpp"

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"
#include "Recluse/Generated/RendererPrograms.hpp"
#include "Recluse/Messaging.hpp"

#include <unordered_map>

namespace Recluse {
namespace PreZ {

GraphicsResource* pSceneDepth = nullptr;

void initialize(GraphicsDevice* pDevice, Engine::SceneBufferDefinitions* pBuffers)
{
    R_ASSERT(pBuffers->gbuffer[Engine::GBuffer_Depth]                   != NULL);
    R_ASSERT(pBuffers->gbuffer[Engine::GBuffer_Depth]->getResource()    != NULL);

    R_DEBUG("PreZ", "Initializing preZ render pass...");

    VertexInputLayout layout = { };
    Runtime::buildVertexInputLayout(pDevice, layout, VertexLayout_PositionOnly);
}


void destroy(GraphicsDevice* pDevice)
{
}


void generate(GraphicsContext* context, Engine::RenderCommandList* pMeshCommandList, U64* keys, U64 sz)
{
    Engine::RenderCommand** pRenderCommands     = pMeshCommandList->getRenderCommands();
    Rect depthRect                              = { };
    depthRect.x         = depthRect.y           = 0.f;

    // We have nothing to do.
    if (sz == 0) return;

    //depthRect.width     = context->getDevice()->getSwapchain()->getDesc().renderWidth;
    //depthRect.height    = context->getDevice()->getSwapchain()->getDesc().renderHeight;

    ResourceViewDescription dsvDesc = { };
    dsvDesc.baseArrayLayer = 0;
    dsvDesc.baseMipLevel = 0;
    dsvDesc.dimension = ResourceViewDimension_2d;
    dsvDesc.format = ResourceFormat_D32_Float;
    dsvDesc.layerCount = 1;
    dsvDesc.mipLevelCount = 1;
    dsvDesc.type = ResourceViewType_DepthStencil;
    ResourceViewId dsv;

    // Set the PreZ pass.
    context->pushState();
    context->clearDepthStencil(ClearFlag_Depth, 1.0f, 0, depthRect);
    context->setCullMode(CullMode_Back);
    context->setFrontFace(FrontFace_Clockwise);
    context->setPolygonMode(PolygonMode_Fill);
    context->bindShaderProgram(ShaderProgram_PreZDepth);
    context->bindRenderTargets(0, nullptr, dsv);

    for (U64 i = 0; i < sz; ++i) 
    {
        U64 key = keys[i];
        Engine::RenderCommand* pRCmd            = pRenderCommands[key];
        PipelineState* pipeline                 = nullptr;
        Engine::DrawableRenderCommand* meshCmd  = nullptr;

        if 
            (
                pRCmd->op != Engine::CommandOp_DrawableIndexedInstanced 
                && pRCmd->op != Engine::CommandOp_DrawableInstanced
            ) 
        {
            continue;
        }

        meshCmd                         = static_cast<Engine::DrawableRenderCommand*>(pRCmd);
        Engine::VertexAttribFlags flags = meshCmd->vertexTypeFlags;

        R_ASSERT_FORMAT(pipeline != NULL, "No pipeline exists for this mesh!");

        context->setInputVertexLayout(VertexLayout_PositionOnly);
        context->bindVertexBuffers(meshCmd->numVertexBuffers, meshCmd->ppVertexBuffers, meshCmd->pOffsets);
        context->setTopology(PrimitiveTopology_TriangleList);

        switch (meshCmd->op) 
        {
            case Engine::CommandOp_DrawableIndexedInstanced:
            {
                Engine::DrawIndexedRenderCommand* pIndexedCmd = 
                    static_cast<Engine::DrawIndexedRenderCommand*>(meshCmd);

                context->bindIndexBuffer
                                (
                                    pIndexedCmd->pIndexBuffer, 
                                    pIndexedCmd->indexType, 
                                    pIndexedCmd->indexType
                                );

                for (U32 submeshIdx = 0; submeshIdx < pIndexedCmd->numSubMeshes; ++submeshIdx) 
                {
                    Engine::IndexedInstancedSubMesh& submesh = pIndexedCmd->pSubMeshes[submeshIdx];
                    
                    context->drawIndexedInstanced
                                        (
                                            submesh.indexCount,
                                            submesh.instanceCount,
                                            submesh.firstIndex,
                                            submesh.vertexOffset,
                                            submesh.firstInstance
                                        );
                }

                break;
            }

            case Engine::CommandOp_DrawableInstanced:
            {
                Engine::DrawRenderCommand* pDrawCmd = static_cast<Engine::DrawRenderCommand*>(meshCmd);

                for (U32 submeshIdx = 0; submeshIdx < pDrawCmd->numSubMeshes; ++submeshIdx) 
                {
                    Engine::InstancedSubMesh& submesh = pDrawCmd->pSubMeshes[submeshIdx];

                    context->drawInstanced
                                        (
                                            submesh.vertexCount,
                                            submesh.instanceCount,
                                            submesh.firstVertex,
                                            submesh.firstInstance
                                        );
                
                }                
                break;
            }
        }
    }

    context->popState();
}
} // PreZ
} // Recluse