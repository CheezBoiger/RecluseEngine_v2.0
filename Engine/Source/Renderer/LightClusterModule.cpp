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


void cullLights(GraphicsContext* context)
{
    R_ASSERT(context != NULL);

    if (context->supportsAsyncCompute()) 
    {
        
    } 
    else 
    {
        
    }
}


void combineForward(GraphicsContext* context, U64* keys, U64 sz)
{
}


void combineDeferred(GraphicsContext* context)
{
}
} // LightCluster
} // Recluse