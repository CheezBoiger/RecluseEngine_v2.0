//

#include "LightClusterModule.hpp"
#include "Recluse/Messaging.hpp"

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


void cullLights(GraphicsCommandList* pList)
{
    R_ASSERT(pList != NULL);

    if (pList->supportsAsyncCompute()) {
        
    } else {
        
    }
}


void combineForward(GraphicsCommandList* pList, U64* keys, U64 sz)
{
}


void combineDeferred(GraphicsCommandList* pList)
{
}
} // LightCluster
} // Recluse