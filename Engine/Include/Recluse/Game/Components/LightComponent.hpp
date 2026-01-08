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
    R_DECLARE_COMPONENT(Light);

    // Need to manually write the default constructor as LightDescription has unions,
    // no way for C++ to know how to automagically construct that.
    Light()
    {
    }

    REDITOR(RATTRIBUTE("visible", "public"),
            RATTRIBUTE("default", null),
            RATTRIBUTE("description", "Light description to describe the light source."))
    LightDescription lightDescription;
};


class LightRegistry : public ECS::ComponentRegistry<Light>
{
public:
    R_DECLARE_COMPONENT_REGISTRY(LightRegistry);
    
    ResultCode onAllocateComponent(const RGUID& owner) override;
    ResultCode onFreeComponent(const RGUID& owner) override;
    U32 queryComponents(Light::Pointer* lights, U32 count) override;
    Light* getComponent(const RGUID& owner) override;
private:
    std::map<RGUID, U32, RGUID::Less> map;
    std::vector<Light*> lights;
    std::queue<U32> freeLights;
};
} // Engine
} // Recluse