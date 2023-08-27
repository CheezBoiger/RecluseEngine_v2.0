//
#include "D3D12Device.hpp"
#include "D3D12PipelineState.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"

#include <unordered_map>
#include <array>

namespace Recluse {

const char* kHlslSemanticPosition   = "POSITION";
const char* kHlslSemanticNormal     = "NORMAL";
const char* kHlslSemanticTexcoord   = "TEXCOORD";
const char* kHlslSemanticTangent    = "TANGENT";


std::unordered_map<Hash64, CpuDescriptorTable> m_cachedCpuDescriptorTables;
std::unordered_map<Hash64, CpuDescriptorTable> m_cachedSamplerTables;


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
        D3D12_ROOT_PARAMETER tableParameters[2] = { };
        std::array<D3D12_DESCRIPTOR_RANGE, 4> ranges = { };
        const Bool hasSamplers = (layout.samplerCount > 0);
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

        tableParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        tableParameters[0].DescriptorTable.NumDescriptorRanges = rangeIdx;
        tableParameters[0].DescriptorTable.pDescriptorRanges = ranges.data();
        tableParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        if (hasSamplers)
        {    
            tableParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            tableParameters[1].DescriptorTable.NumDescriptorRanges = 1;
            tableParameters[1].DescriptorTable.pDescriptorRanges = &ranges[Math::clamp((U32)(rangeIdx - 1), (U32)0, (U32)ranges.size())];
            tableParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }
        desc.NumParameters = hasSamplers ? 2 : 1;
        desc.NumStaticSamplers = 0;
        desc.pParameters = tableParameters;
        desc.pStaticSamplers = nullptr;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // TODO: None for now, but we might want to try and optimize this?

        ID3DBlob* pSignature;
        ID3DBlob* pErrorBlob;
        HRESULT result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pErrorBlob);
        if (SUCCEEDED(result))
        {
            result = pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pRootSig);
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


CpuDescriptorTable makeDescriptorSrvCbvUavTable(D3D12Device* pDevice, const RootSigLayout& layout, const RootSigResourceTable& resourceTable)
{
    DescriptorHeapAllocationManager* pManager = pDevice->getDescriptorHeapManager();
    Hash64 hash = 0;
    const U32 descriptorCount = layout.cbvCount + layout.srvCount + layout.uavCount;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles(descriptorCount);
    U32 i = 0;
    for (U32 j = 0; j < layout.cbvCount; ++j)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = resourceTable.cbvs[j];
        if (handle.ptr != DescriptorTable::invalidCpuAddress.ptr)
            handles[i++] = resourceTable.cbvs[j];
        else
            handles[i++] = pManager->nullCbvDescriptor();
    }
    for (U32 j = 0; j < layout.srvCount; ++j)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = resourceTable.srvs[j];
        if (handle.ptr != DescriptorTable::invalidCpuAddress.ptr)
            handles[i++] = resourceTable.srvs[j];
        else
            handles[i++] = pManager->nullSrvDescriptor();
    }
    for (U32 j = 0; j < layout.uavCount; ++j)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = resourceTable.uavs[j];
        if (handle.ptr != DescriptorTable::invalidCpuAddress.ptr)
            handles[i++] = resourceTable.uavs[j];
        else
            handles[i++] = pManager->nullUavDescriptor();
    }
    hash = recluseHashFast(handles.data(), sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * handles.size());
    auto iter = m_cachedCpuDescriptorTables.find(hash);
    if (iter == m_cachedCpuDescriptorTables.end())
    {
        CpuDescriptorTable table = pManager->copyDescriptorsToTable(CpuHeapType_CbvSrvUav, handles.data(), descriptorCount);
        m_cachedCpuDescriptorTables.insert(std::make_pair(hash, table));
        return table; 
    }
    else
    {
        return iter->second;
    }
}


CpuDescriptorTable              makeDescriptorSamplertable(D3D12Device* pDevice, const RootSigLayout& layout, const RootSigResourceTable& resourceTable)
{
    DescriptorHeapAllocationManager* pManager = pDevice->getDescriptorHeapManager();
    const U32 descriptorCount = layout.samplerCount;
    Hash64 hash = recluseHashFast(resourceTable.samplers, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * descriptorCount);
    auto iter = m_cachedSamplerTables.find(hash);
    if (iter == m_cachedSamplerTables.end())
    {
        CpuDescriptorTable table = pManager->copyDescriptorsToTable(CpuHeapType_CbvSrvUav, resourceTable.samplers, descriptorCount);
        m_cachedSamplerTables.insert(std::make_pair(hash, table));
        return table; 
    }
    else
    {
        return iter->second;
    }
}


void cleanUpRootSigs()
{
    for (auto rootsig : m_rootSignatures)
    {
        rootsig.second->Release();
    }

    m_rootSignatures.clear();
}


void resetTableHeaps(D3D12Device* pDevice)
{
    DescriptorHeapAllocationManager* pManager = pDevice->getDescriptorHeapManager();
    pManager->resetCpuTableHeaps();
    m_cachedCpuDescriptorTables.clear();
    m_cachedSamplerTables.clear();
}
} // Pipelines
} // Recluse