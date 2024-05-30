//
#include "Recluse/Game/Systems/LightSystem.hpp"
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Game/Components/LightComponent.hpp"
#include "Recluse/Renderer/Renderer.hpp"

namespace Recluse {
namespace Engine {


void LightSystem::onUpdate(Engine::Scene* scene, const RealtimeTick& tick)
{
    ECS::Registry<Light>* lightRegistry = scene->getRegistry<Light>();
    if (lightRegistry)
    {
        std::vector<Light*> lights = lightRegistry->getAllComponents();
        Renderer* renderer = Renderer::getMain();
        for (U32 lightIdx = 0; lightIdx < lights.size(); ++lightIdx)
        {
            Light* light = lights[lightIdx];
            if (light->isEnabled())
                renderer->pushLight(light->lightDescription);
        }
    }
}
} // Engine
} // Recluse