//
#include "Recluse/Game/Components/LightComponent.hpp"

namespace Recluse {
namespace Engine {


ResultCode LightRegistry::onAllocateComponent(const RGUID& owner)
{
    auto it = map.find(owner);
    if (it == map.end())
    {
        lights.push_back(new Light());
        U32 id = lights.size() - 1;
        map.insert(std::make_pair(owner, id));
        return RecluseResult_Ok;
    }
    return RecluseResult_AlreadyExists;
}


ResultCode LightRegistry::onFreeComponent(const RGUID& owner)
{
    auto it = map.find(owner);
    if (it == map.end())
    {
        return RecluseResult_NotFound;
    }
    U32 id = it->second;
    freeLights.push(id);
    map.erase(it);
    delete lights[id];
    return RecluseResult_Ok;
}


U32 LightRegistry::queryComponents(Light::Pointer* lights, U32 count)
{
    if (lights)
    {
        for (U32 i = 0; i < this->lights.size(); ++i)
            lights[i] = this->lights[i];
    }
    return static_cast<U32>(this->lights.size());
}


Light* LightRegistry::getComponent(const RGUID& owner)
{
    auto it = map.find(owner);
    if (it == map.end())
    {
        return nullptr;
    }
    return lights[it->second];
}
} // Engine
} // Recluse