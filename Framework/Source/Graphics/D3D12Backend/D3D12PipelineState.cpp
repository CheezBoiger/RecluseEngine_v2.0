//
#include "D3D12PipelineState.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Messaging.hpp"

#include <unordered_map>

namespace Recluse {

const char* kHlslSemanticPosition   = "POSITION";
const char* kHlslSemanticNormal     = "NORMAL";
const char* kHlslSemanticTexcoord   = "TEXCOORD";
const char* kHlslSemanticTangent    = "TANGENT";


namespace Pipelines {


std::unordered_map<PipelineStateId, ID3D12PipelineState*> g_pipelineStateMap;


static PipelineStateId serializePipelineState(const PipelineStateObject& pipelineState)
{
    return recluseHashFast(&pipelineState, sizeof(PipelineStateObject));
}


static ID3D12PipelineState* createGraphicsPipelineState(const PipelineStateObject& pipelineState)
{
    return nullptr;
}


static ID3D12PipelineState* createComputePipelineState(const PipelineStateObject& pipelineState)
{
    return nullptr;
}


static ID3D12PipelineState* createPipelineState(const PipelineStateObject& pipelineState)
{
    ID3D12PipelineState* createdPipelineState = nullptr;
    switch (pipelineState.pipelineType)
    {
        case BindType_Graphics:
            createdPipelineState = createGraphicsPipelineState(pipelineState);
            break;
        case BindType_RayTrace:
            break;
        case BindType_Compute:
            createdPipelineState = createComputePipelineState(pipelineState);
            break;
        default:
            break;        
    }
    R_ASSERT(createdPipelineState != nullptr);
    return createdPipelineState;
}


ID3D12PipelineState* makePipelineState(D3D12Context* pContext, const PipelineStateObject& pipelineState)
{
    ID3D12PipelineState* retrievedPipelineState = nullptr;
    PipelineStateId pipelineId = serializePipelineState(pipelineState);
    auto iter = g_pipelineStateMap.find(pipelineId);
    if (iter == g_pipelineStateMap.end())
    {
        // We didn't find a similar pipeline state, need to create a new one.
        
    }
    retrievedPipelineState = iter->second;
    return retrievedPipelineState;
}
} // Pipelines
} // Recluse