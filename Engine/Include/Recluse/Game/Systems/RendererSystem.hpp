//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Game/GameSystem.hpp"

#include "Recluse/Game/Components/RendererComponent.hpp"

namespace Recluse {


class RendererSystem : public ECS::System
{
public:

    virtual         ~RendererSystem() { }
};
} // Recluse