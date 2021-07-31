//
#pragma once

#include "Recluse/Types.hpp"

#include <vector>
#include <unordered_map>

namespace Recluse {

class GraphicsDevice;
class GraphicsResource;
class Bounds3D;
class Material;

namespace Engine {



class VertexBuffer {
public:
    VertexBuffer();
    
    void initialize(GraphicsDevice* pDevice, U64 totalVertices, U64 perVertexSzBytes);
    void destroy(GraphicsDevice* pDevice);

    GraphicsResource* get() const { return m_pResource; }
private:
    GraphicsResource* m_pResource;    
};

struct SubMesh {
    U64         offset;
    U64         numVertices;
    Material*   pMaterial;
};

class Mesh {
public:

    VertexBuffer& getVertexBuffer() { return m_vertexBuffer; }

    const std::vector<SubMesh>& getSubMeshes() { return m_submeshes; };

private:
    std::unordered_map<std::string, SubMesh*>   m_quickMap;
    std::vector<SubMesh>                        m_submeshes;
    VertexBuffer                                m_vertexBuffer;
};
} // Engine
} // Recluse