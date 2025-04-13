//
#pragma once

#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Pipeline/Importer.hpp"

#include <vector>

#include "ReclusePipeline_exports.hpp"

namespace Recluse {
namespace Pipeline {
namespace Builder {


// SceneBuilder builds entire scenes from files, but requires importers.
class ReclusePipeline_PUBLIC_API SceneBuilder : public Serializable
{
public:
    SceneBuilder() { }
    virtual ~SceneBuilder() { }

    static SceneBuilder* create(Importer* importer);

    virtual ResultCode  serialize(Archive* archive) const override;
    virtual ResultCode  deserialize(Archive* archive) override;

    Engine::Scene*      getScene(const std::string& name);
    Engine::Scene*      getScene(u32 index);
private:  
    std::vector<Engine::Scene*> m_scenes;  
};
} // Builder
} // Pipeline
} // Recluse