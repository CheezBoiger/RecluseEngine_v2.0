//
#pragma once

#include "Recluse/Types.hpp"
#include <vector>

namespace Recluse {

class GraphicsDevice;
class GraphicsResource;
class Bounds3D;

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
    U64 offset;
    U64 numVertices;
    
};

class Mesh {
public:


private:
    
    std::vector<SubMesh> m_submeshes;
    VertexBuffer m_vertexBuffer;
};
} // Engine
} // Recluse