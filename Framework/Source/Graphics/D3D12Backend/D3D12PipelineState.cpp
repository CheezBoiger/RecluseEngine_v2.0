//
#include "D3D12Device.hpp"
#include "D3D12PipelineState.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Messaging.hpp"

#include <unordered_map>
#include <array>

namespace Recluse {

const char* kHlslSemanticPosition   = "POSITION";
const char* kHlslSemanticNormal     = "NORMAL";
const char* kHlslSemanticTexcoord   = "TEXCOORD";
const char* kHlslSemanticTangent    = "TANGENT";


namespace Pipelines {


std::unordered_map<PipelineStateId, ID3D12PipelineState*> g_pipelineStateMap;
std::unordered_map<Hash64, ID3D12RootSignature*> m_rootSignatures;


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



// Creates a root signature under one descriptor table.
// The descriptor table must be laid out as such:
//          Descriptor Table ->
//              -> CBV 
//              -> SRV
//              -> UAV
//              -> Samplers
// Each layout can assign multple cbvs, srvs, uavs, and samplers, as long as they 
// are laid out in linear fashion.
R_INTERNAL
ID3D12RootSignature* internalCreateRootSignatureWithTable(ID3D12Device* pDevice, const RootSigLayout& layout)
{
    ID3D12RootSignature* pRootSig = nullptr;
    const U32 totalDescriptors = layout.cbvCount + layout.samplerCount + layout.srvCount + layout.uavCount;

    if (totalDescriptors)
    {
        D3D12_ROOT_SIGNATURE_DESC desc = { };
        // For CbvSrvUavs and Samplers.
        D3D12_ROOT_PARAMETER tableParameters = { };
        std::array<D3D12_DESCRIPTOR_RANGE, 4> ranges = { };
        
        U32 rangeIdx = 0;
        U32 offsetInDescriptors = 0;
        if (layout.cbvCount > 0)
        {
            D3D12_DESCRIPTOR_RANGE& range = ranges[rangeIdx++];
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            range.NumDescriptors = layout.cbvCount;
            range.BaseShaderRegister = 0;
            range.RegisterSpace = 0;
            range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
            offsetInDescriptors += layout.cbvCount;
        }
        if (layout.srvCount > 0)
        {
            D3D12_DESCRIPTOR_RANGE& range = ranges[rangeIdx++];
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.NumDescriptors = layout.srvCount;
            range.BaseShaderRegister = 0;
            range.RegisterSpace = 0;
            range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
            offsetInDescriptors += layout.srvCount;
        }
        if (layout.uavCount > 0)
        {
            D3D12_DESCRIPTOR_RANGE& range = ranges[rangeIdx++];
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            range.NumDescriptors = layout.uavCount;
            range.BaseShaderRegister = 0;
            range.RegisterSpace = 0;
            range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
            offsetInDescriptors += layout.uavCount;
        }
        if (layout.samplerCount > 0)
        {
            D3D12_DESCRIPTOR_RANGE& range = ranges[rangeIdx++];
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            range.NumDescriptors = layout.samplerCount;
            range.BaseShaderRegister = 0;
            range.RegisterSpace = 0;
            range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
            offsetInDescriptors += layout.samplerCount;
        }

        tableParameters.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        tableParameters.DescriptorTable.NumDescriptorRanges = rangeIdx;
        tableParameters.DescriptorTable.pDescriptorRanges = ranges.data();
        tableParameters.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        desc.NumParameters = 1;
        desc.NumStaticSamplers = 0;
        desc.pParameters = &tableParameters;
        desc.pStaticSamplers = nullptr;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // TODO: None for now, but we might want to try and optimize this?

        ID3DBlob* pSignature;
        ID3DBlob* pErrorBlob;
        HRESULT result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pErrorBlob);
        if (SUCCEEDED(result))
        {
            result = pDevice->CreateRootSignature(0, pSignature, pSignature->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pRootSig);
            if (FAILED(result))
            {
                R_ERROR(R_CHANNEL_D3D12, "Failed to create RootSig! Error code: %x08", result);
            }
        }
        else
        {
            R_ERROR(R_CHANNEL_D3D12, "Failed to serialize RootSig, Error: %s", (const char*)pErrorBlob->GetBufferPointer());
        }
        pSignature->Release();
        pSignature->Release();
    }
    return pRootSig;
}


R_INTERNAL
ID3D12RootSignature* internalCreateRootSignatureSeparateParameters(ID3D12Device* pDevice, const RootSigLayout& layout)
{
    R_NO_IMPL();
    return nullptr;
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


ID3D12RootSignature* makeRootSignature(D3D12Device* pDevice, const RootSigLayout& layout)
{
    ID3D12RootSignature* rootSignature = nullptr;
    Hash64 hash = recluseHashFast(&layout, sizeof(RootSigLayout));
    auto iter = m_rootSignatures.find(hash);
    if (iter == m_rootSignatures.end())
    {
        rootSignature = internalCreateRootSignatureWithTable(pDevice->get(), layout);
        m_rootSignatures.insert(std::make_pair(hash, rootSignature));
    }
    else
    {
        rootSignature = iter->second;
    }
    return rootSignature;
}
} // Pipelines
} // Recluse