//

#include "LightClusterModule.hpp"

#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

namespace Recluse {
namespace LightCluster {


PipelineState* pComputePipelineState    = nullptr;

GraphicsResource* pLightGrid            = nullptr;
GraphicsResource* pLightIndices         = nullptr;


void cull(GraphicsCommandList* pList)
{
    
}
} // LightCluster
} // Recluse