//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Renderer/SceneView.hpp"
#include "Recluse/Renderer/Mesh.hpp"
#include "Recluse/Renderer/Material.hpp"

namespace Recluse {
namespace Engine {


struct SubmeshRenderCommand {
    Material* pMaterial;
    SubMesh** ppSubmeshes;
    U32       meshIndex;
};

struct RenderCommand {
    Mesh**                  pVertexBuffers;
    U32                     numVertexBuffers;
    TransformPerMesh*       pTransformPerMesh;
    SubmeshRenderCommand    submeshes;
};



class RenderCommandList {
public:
    
};
} // Engine
} // Recluse