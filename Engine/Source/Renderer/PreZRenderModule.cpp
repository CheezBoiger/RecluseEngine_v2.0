//
#include "PreZRenderModule.hpp"

#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Renderer/RendererResources.hpp"

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Graphics/ShaderProgramBuilder.hpp"
#include "Recluse/Generated/RendererPrograms.hpp"
#include "Recluse/Messaging.hpp"

#include <unordered_map>

namespace Recluse {
namespace PreZ {

GraphicsResourceView* pSceneDepthView = nullptr;

std::unordered_map<Engine::VertexAttribFlags, PipelineState*> pipelines;
RenderPass* pPreZPass = nullptr;

void initialize(GraphicsDevice* pDevice, Engine::SceneBufferDefinitions* pBuffers)
{
    R_ASSERT(pBuffers->gbuffer[Engine::GBuffer_Depth]                   != NULL);
    R_ASSERT(pBuffers->gbuffer[Engine::GBuffer_Depth]->getResource()    != NULL);
    R_ASSERT(pBuffers->gbufferViews[Engine::GBuffer_Depth]->getView()   != NULL);

    R_DEBUG("PreZ", "Initializing preZ render pass...");

    Builder::ShaderProgramDescription description = { };
    description.pipelineType = BindType_Graphics;
    description.language = ShaderLang_Hlsl;
    description.graphics.vs = Engine::getBinaryShaderPath("Geometry/VertFactoryStatic").c_str();
    description.graphics.vsName = "main";
    Builder::buildShaderProgramDefinitions(description, ShaderProgram_PreZDepth, ShaderIntermediateCode_Spirv);
    Builder::Runtime::buildShaderProgram(pDevice, ShaderProgram_PreZDepth);
    Builder::releaseShaderProgramDefinition(ShaderProgram_PreZDepth);

    VertexInputLayout layout = { };
    Builder::Runtime::buildVertexInputLayout(pDevice, layout, VertexLayout_PositionOnly);
}


void destroy(GraphicsDevice* pDevice)
{
}


void generate(GraphicsContext* context, Engine::RenderCommandList* pMeshCommandList, U64* keys, U64 sz)
{
    Engine::RenderCommand** pRenderCommands     = pMeshCommandList->getRenderCommands();
    const GraphicsResourceDescription& depth    = pSceneDepthView->getResource()->getDesc();
    Rect depthRect                              = { };
    depthRect.x         = depthRect.y           = 0.f;

    depthRect.width     = depth.width;
    depthRect.height    = depth.height;

    // Set the PreZ pass.
    context->setShaderProgram(ShaderProgram_PreZDepth);
    context->clearDepthStencil(1.0f, 0, depthRect);
    context->setCullMode(CullMode_Back);
    context->setFrontFace(FrontFace_Clockwise);
    context->setPolygonMode(PolygonMode_Fill);

    for (U64 i = 0; i < sz; ++i) 
    {
        U64 key = keys[i];
        Engine::RenderCommand* pRCmd            = pRenderCommands[key];
        PipelineState* pipeline                 = nullptr;
        Engine::DrawableRenderCommand* meshCmd  = nullptr;

        if 
            (
                meshCmd->op != Engine::CommandOp_DrawableIndexedInstanced 
                || meshCmd->op != Engine::CommandOp_DrawableInstanced
            ) 
        {
            continue;
        }

        meshCmd                         = static_cast<Engine::DrawableRenderCommand*>(pRCmd);
        Engine::VertexAttribFlags flags = meshCmd->vertexTypeFlags;
        pipeline                        = pipelines[flags];

        R_ASSERT_MSG(pipeline != NULL, "No pipeline exists for this mesh!");

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
}
} // PreZ
} // Recluse