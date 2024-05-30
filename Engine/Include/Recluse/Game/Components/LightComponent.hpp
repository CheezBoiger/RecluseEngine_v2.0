//
#pragma once

#include "Recluse/Game/Component.hpp"
#include "Recluse/Generated/Common/Common.hpp"
#include <vector>
#include <map>
namespace Recluse {
namespace Engine {


class Light : public ECS::Component
{
public:
    R_COMPONENT_DECLARE(Light);
    
    LightDescription lightDescription;
};


class LightRegistry : public ECS::Registry<Light>
{
public:
    R_COMPONENT_REGISTRY_DECLARE(LightRegistry);
    

private:
    std::map<RGUID, Light, RGUID::Less> map;
    std::vector<Light*> lights;
};
} // Engine
} // Recluse