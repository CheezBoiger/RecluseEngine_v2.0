//
#pragma once

#include "Recluse/Game/Component.hpp"
#include "Recluse/Generated/Common/Common.hpp"
#include <vector>
#include <map>
#include <queue>
namespace Recluse {
namespace Engine {


class Light : public ECS::Component
{
public:
    R_COMPONENT_DECLARE(Light);

    // Need to manually write the default constructor as LightDescription has unions,
    // no way for C++ to know how to automagically construct that.
    Light()
        : lightDescription({ })
    {
    };

    REDITOR(RATTRIBUTE("visible", "public"),
            RATTRIBUTE("default", null),
            RATTRIBUTE("description", "Light description to describe the light source."))
    LightDescription lightDescription;
};


class LightRegistry : public ECS::ComponentRegistry<Light>
{
public:
    R_COMPONENT_REGISTRY_DECLARE(LightRegistry);
    
    ResultCode onAllocateComponent(const RGUID& owner) override;
    ResultCode onFreeComponent(const RGUID& owner) override;
    std::vector<Light*> getAllComponents() override;
    Light* getComponent(const RGUID& owner) override;
private:
    std::map<RGUID, U32, RGUID::Less> map;
    std::vector<Light*> lights;
    std::queue<U32> freeLights;
};
} // Engine
} // Recluse