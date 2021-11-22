//
#pragma once

#include "Recluse/Graphics/CommandList.hpp"

#include "Recluse/Renderer/Mesh.hpp"
#include "Recluse/Renderer/RenderCommand.hpp"
#include "Recluse/Renderer/RendererResources.hpp"

namespace Recluse {
namespace PreZ {

void initialize(GraphicsDevice* pDevice, Engine::SceneBufferDefinitions* pBuffers);
void destroy(GraphicsDevice* pDevice);
void generate(GraphicsCommandList* pCommandList, Engine::RenderCommandList* pMeshCommandList, U64* keys, U64 sz);
} // PreZ
} // Recluse