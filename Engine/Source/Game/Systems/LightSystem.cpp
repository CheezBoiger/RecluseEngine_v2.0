//
#include "Recluse/Game/Systems/LightSystem.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/Components/LightComponent.hpp"
#include "Recluse/Renderer/Renderer.hpp"

namespace Recluse {
namespace Engine {


void LightSystem::onUpdate(ECS::Registry* registry, const RealtimeTick& tick, ECS::EntityHierarchy* hierarchy)
{
    std::vector<Light*> lights = obtainComponents(registry);
    RendererModule* renderer = RendererModule::getMain();
    for (U32 lightIdx = 0; lightIdx < lights.size(); ++lightIdx)
    {
        Light* light = lights[lightIdx];
        if (light->isEnabled())
            renderer->pushLight(light->lightDescription);
    }
}
} // Engine
} // Recluse