//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Renderer/RendererResources.hpp"
#include <vector>
#include <map>

namespace Recluse {

class GraphicsDevice;
class GraphicsResource;
class DescriptorSetLayout;
class GraphicsQueue;
class DescriptorSet;
class Bounds3D;
class Material;

namespace Engine {


// Per mesh information.
struct R_EXPORT PerMeshTransform {
    Matrix44 world;
    Matrix44 worldToViewClip;
    Matrix44 n;
};


struct R_EXPORT SubMesh {
    std::string name;
    U64         offset;
    U64         numVertices;
};


class Mesh {
public:
    virtual ~Mesh() { }

    R_EXPORT Mesh()
        : m_pVertexBuffer(nullptr)
        , m_pIndexBuffer(nullptr) { }

    R_EXPORT ErrType initialize(VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer);

    R_EXPORT VertexBuffer* getVertexBuffer() { return m_pVertexBuffer; }
    R_EXPORT IndexBuffer* getIndexBuffer() { return m_pIndexBuffer; }

    R_EXPORT const std::vector<SubMesh*>& getSubMeshes() { return m_submeshes; };

    R_EXPORT void addSubmeshes(U32 numSubmeshes, SubMesh* pSubmeshes) {
        for (U32 i = 0; i < numSubmeshes; ++i) { 
            m_subMeshMap[pSubmeshes[i].name] = pSubmeshes[i];
            m_submeshes.push_back(&m_subMeshMap[pSubmeshes[i].name]);
        }
    }

private:
    std::map<std::string, SubMesh>  m_subMeshMap;
    std::vector<SubMesh*>           m_submeshes;
    VertexBuffer*                   m_pVertexBuffer;
    IndexBuffer*                    m_pIndexBuffer;
};


typedef GPUBuffer PerInstancedMeshBuffer;

class MeshInstanced {
public:
    
    ErrType initializeInstanced();

private:
    
    PerInstancedMeshBuffer m_perInstancedMeshBuffer;
};
} // Engine
} // Recluse