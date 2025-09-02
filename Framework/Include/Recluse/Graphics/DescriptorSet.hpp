//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

#include <array>

namespace Recluse {


class GraphicsResource;
class GraphicsResourceView;
class GraphicsSampler;


template<uint numResources>
class BindingResourceTableLayout
{
public:
    struct BindingDescription
    {
        ResourceViewType viewType;
    };

    std::array<BindingDescription, numResources> bindings;
};

typedef U64 BindingResourceTableId;
} // Recluse