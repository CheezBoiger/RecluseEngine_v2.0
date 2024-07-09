//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/Renderer/Mesh.hpp"
#include "Recluse/RGUID.hpp"

#include <vector>

namespace Recluse {
namespace Pipeline {
namespace Builder {


// Mesh Builder
class R_PUBLIC_API MeshBuilder : public Serializable
{
public:
    // 
    enum 
    {
        Mesh_Optimize = (1<<0),
        Mesh_Quantize = (1<<1),
        Mesh_Simplify = (1<<2)
    };
    typedef U32 MeshBuilderFlags;

    MeshBuilder(const std::string& name = "", const RGUID& rguid);

    ResultCode build(const std::string& meshPath, MeshBuilderFlags flags);

    ResultCode serialize(Archive* archive) const override;
    ResultCode deserialize(Archive* archive) override;

private:
    // Name of the mesh.
    std::string                             m_name;
    // Id of this mesh.
    RGUID                                   m_guid;

    // Engine submeshes.
    std::map<std::string, Engine::SubMesh>  m_submeshMap;
    std::vector<Engine::SubMesh*>           m_submeshes;

    // Data related to the mesh is arbitrary, especially if we end up quantizing.
    void* m_positionData;
    void* m_normalsData;
    void* m_texcoordData;
};
} // Builder
} // Pipeline
} // Recluse