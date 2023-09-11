//
#include "D3D12Device.hpp"
#include "D3D12PipelineState.hpp"
#include "D3D12RenderPass.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include "D3D12ShaderCache.hpp"
#include "D3D12RenderPass.hpp"

#include <map>
#include <unordered_map>
#include <array>

namespace Recluse {


std::map<Semantic, const char*> g_semanticMap = {
    { Semantic_Position, "POSITION" },
    { Semantic_Normal, "NORMAL" },
    { Semantic_Texcoord, "TEXCOORD" },
    { Semantic_Tangent, "TANGENT" },
    { Semantic_Binormal, "BINORMAL" },
    { Semantic_Color, "COLOR" },
    { Semantic_TessFactor, "TESSFACTOR" }
};


std::unordered_map<Hash64, CpuDescriptorTable> m_cachedCpuDescriptorTables;
std::unordered_map<Hash64, CpuDescriptorTable> m_cachedSamplerTables;


namespace Pipelines {


std::unordered_map<PipelineStateId, ID3D12PipelineState*> g_pipelineStateMap;
std::unordered_map<Hash64, ID3D12RootSignature*> m_rootSignatures;


namespace VertexInputs {


std::unordered_map<VertexInputLayoutId, D3DVertexInput> g_vertexLayouts;


D3D12_INPUT_CLASSIFICATION getInputClassification(InputRate inputRate)
{
    switch (inputRate)
    {
        case InputRate_PerInstance: return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        default:
        case InputRate_PerVertex:   return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    }
}


ResultCode make(VertexInputLayoutId id, const VertexInputLayout& layout)
{
    R_ASSERT(layout.numVertexBindings < VertexInputLayout::VertexInputLayout_BindingCount);
    auto iter = g_vertexLayouts.find(id);
    if (iter != g_vertexLayouts.end())
    {
        return RecluseResult_AlreadyExists;
    }
    else
    {
        D3DVertexInput inputs;
        for (U32 i = 0; i < layout.numVertexBindings; ++i)
        {
            const VertexBinding& vertexBinding = layout.vertexBindings[i];
            U32 inputSlot = vertexBinding.binding;
            D3D12_INPUT_CLASSIFICATION classification = getInputClassification(vertexBinding.inputRate);
            UINT strideBytes = 0;
            for (U32 attribIndex = 0; attribIndex < vertexBinding.numVertexAttributes; ++attribIndex)
            {
                VertexAttribute& attrib             = vertexBinding.pVertexAttributes[attribIndex];
                D3D12_INPUT_ELEMENT_DESC element    = { };
                element.InputSlot                   = inputSlot;
                element.InputSlotClass              = classification;
                element.Format                      = Dxgi::getNativeFormat(attrib.format);
                element.InstanceDataStepRate        = classification != D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ? 1 : 0;
                element.SemanticIndex               = attrib.semanticIndex;
                element.SemanticName                = g_semanticMap[attrib.semantic];
                if (attrib.offsetBytes == VertexAttribute::OffsetAppend)
                {
                    element.AlignedByteOffset           = D3D12_APPEND_ALIGNED_ELEMENT;
                }
                else
                {
                    element.AlignedByteOffset           = attrib.offsetBytes;
                } 
                strideBytes += static_cast<UINT>(Dxgi::getNativeFormatSize(element.Format));
                inputs.elements.push_back(element);
            }
            inputs.vertexByteStrides.push_back(vertexBinding.stride == 0 ? strideBytes : vertexBinding.stride);
        }

        g_vertexLayouts.insert(std::make_pair(id, inputs));
    }
    return RecluseResult_Ok;
}


ResultCode unload(VertexInputLayoutId id)
{
    auto& iter = g_vertexLayouts.find(id);
    if (iter != g_vertexLayouts.end())
    {
        g_vertexLayouts.erase(iter);
        return RecluseResult_Ok;
    }
    return RecluseResult_NotFound;
}


Bool unloadAll()
{
    g_vertexLayouts.clear();
    return true;
}


D3DVertexInput* obtain(VertexInputLayoutId layoutId)
{
    auto& iter = g_vertexLayouts.find(layoutId);
    if (iter != g_vertexLayouts.end())
    {
        return &iter->second;
    }
    else
    {
        return nullptr;
    }
}
} // VertexInputs


R_INTERNAL
D3D12_FILL_MODE getFillMode(PolygonMode mode)
{
    switch (mode)
    {
        case PolygonMode_Line:      return D3D12_FILL_MODE_WIREFRAME;
        case PolygonMode_Point:     return D3D12_FILL_MODE_SOLID;
        default:
        case PolygonMode_Fill:      return D3D12_FILL_MODE_SOLID;
    }
}


R_INTERNAL 
D3D12_CULL_MODE getCullMode(CullMode mode)
{
    switch (mode)
    {
        case CullMode_Back:             return D3D12_CULL_MODE_BACK;
        case CullMode_Front:            return D3D12_CULL_MODE_FRONT;
        case CullMode_FrontAndBack:     return D3D12_CULL_MODE_NONE;
        default:
        case CullMode_None:             return D3D12_CULL_MODE_NONE;
    }
}


R_INTERNAL 
PipelineStateId serializePipelineState(const PipelineStateObject& pipelineState)
{
    return recluseHashFast(&pipelineState, sizeof(PipelineStateObject));
}


D3D12_DEPTH_STENCILOP_DESC fillStencilState(const StencilOpState& apiState)
{
    D3D12_DEPTH_STENCILOP_DESC descriptor   = { };
    descriptor.StencilDepthFailOp           = getStencilOp(apiState.depthFailOp);
    descriptor.StencilFailOp                = getStencilOp(apiState.failOp);
    descriptor.StencilPassOp                = getStencilOp(apiState.passOp);
    descriptor.StencilFunc                  = getNativeComparisonFunction(apiState.compareOp);
    return descriptor;
}


R_INTERNAL 
ID3D12PipelineState* createGraphicsPipelineState(U32 nodeMask, ID3D12Device* pDevice, const D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = { };
    desc.pRootSignature = pipelineState.rootSignature;
    desc.NodeMask = 0;
    desc.NumRenderTargets = pipelineState.graphics.numRenderTargets;
    desc.IBStripCutValue = pipelineState.graphics.indexStripCut;
    desc.PrimitiveTopologyType = pipelineState.graphics.topologyType;
    D3D12RenderPass* pRenderPass = pipelineState.graphics.pRenderPass;
    for (U32 i = 0; i < pipelineState.graphics.numRenderTargets; ++i)
    {
        desc.RTVFormats[i] = pipelineState.graphics.pRenderPass->getRtvFormat(i);
    }
    desc.DSVFormat = pipelineState.graphics.pRenderPass->getDsvFormat();

    R_ASSERT(program->graphics.vsBytecode);
    
    desc.VS.pShaderBytecode = program->graphics.vsBytecode->GetBufferPointer();
    desc.VS.BytecodeLength = program->graphics.vsBytecode->GetBufferSize();
    
    if (program->graphics.psBytecode)
    {
        desc.PS.pShaderBytecode = program->graphics.psBytecode->GetBufferPointer();
        desc.PS.BytecodeLength = program->graphics.psBytecode->GetBufferSize();
    }

    if (program->graphics.hsBytecode)
    {
        desc.HS.pShaderBytecode = program->graphics.hsBytecode->GetBufferPointer();
        desc.HS.BytecodeLength = program->graphics.hsBytecode->GetBufferSize();
    }

    if (program->graphics.dsBytecode)
    {
        desc.DS.pShaderBytecode = program->graphics.dsBytecode->GetBufferPointer();
        desc.DS.BytecodeLength = program->graphics.dsBytecode->GetBufferSize();
    }

    if (program->graphics.gsBytecode)
    {
        desc.GS.pShaderBytecode = program->graphics.gsBytecode->GetBufferPointer();
        desc.GS.BytecodeLength = program->graphics.gsBytecode->GetBufferSize();
    }

    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    desc.RasterizerState.AntialiasedLineEnable = pipelineState.graphics.antiAliasedLineEnable;
    desc.RasterizerState.FillMode = getFillMode(pipelineState.graphics.polygonMode);
    desc.RasterizerState.CullMode = getCullMode(pipelineState.graphics.cullMode);
    desc.RasterizerState.DepthClipEnable = pipelineState.graphics.depthClampEnable;
    desc.RasterizerState.DepthBias  = pipelineState.graphics.depthBiasEnable ? 1 : 0;
    desc.RasterizerState.FrontCounterClockwise = (pipelineState.graphics.frontFace == FrontFace_CounterClockwise ? true : false);
    desc.RasterizerState.MultisampleEnable = false;
    desc.RasterizerState.AntialiasedLineEnable = false;
    desc.RasterizerState.SlopeScaledDepthBias = 0.0f;

    desc.DepthStencilState.DepthEnable = pipelineState.graphics.depthStencil.depthTestEnable;
    desc.DepthStencilState.StencilEnable = pipelineState.graphics.depthStencil.stencilTestEnable;
    desc.DepthStencilState.DepthWriteMask = pipelineState.graphics.depthStencil.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    desc.DepthStencilState.DepthFunc = getNativeComparisonFunction(pipelineState.graphics.depthStencil.depthCompareOp);
    desc.DepthStencilState.StencilReadMask = pipelineState.graphics.depthStencil.stencilReadMask;
    desc.DepthStencilState.StencilWriteMask = pipelineState.graphics.depthStencil.stencilWriteMask;
    desc.DepthStencilState.FrontFace = fillStencilState(pipelineState.graphics.depthStencil.front);
    desc.DepthStencilState.BackFace = fillStencilState(pipelineState.graphics.depthStencil.back);
    Bool independentBlendEnable = false;
    for (U32 i = 0; i < pipelineState.graphics.numRenderTargets; ++i)
    {
        D3D12_RENDER_TARGET_BLEND_DESC& rtBlend     = desc.BlendState.RenderTarget[i];
        const RenderTargetBlendState& rtBlendState  = pipelineState.graphics.blendState.attachments[i];
        rtBlend.RenderTargetWriteMask               = rtBlendState.colorWriteMask;
        rtBlend.BlendEnable                         = rtBlendState.blendEnable;
        rtBlend.BlendOp                             = getBlendOp(rtBlendState.colorBlendOp);
        rtBlend.BlendOpAlpha                        = getBlendOp(rtBlendState.alphaBlendOp);
        rtBlend.LogicOp                             = getLogicOp(pipelineState.graphics.blendState.logicOp);
        rtBlend.DestBlend                           = getBlendFactor(rtBlendState.dstColorBlendFactor);
        rtBlend.DestBlendAlpha                      = getBlendFactor(rtBlendState.dstAlphaBlendFactor);
        rtBlend.SrcBlend                            = getBlendFactor(rtBlendState.srcColorBlendFactor);
        rtBlend.SrcBlendAlpha                       = getBlendFactor(rtBlendState.srcAlphaBlendFactor);
        independentBlendEnable |= rtBlend.BlendEnable;
    }

    if (pRenderPass)
    {
        for (U32 i = 0; i < pipelineState.graphics.numRenderTargets; ++i)
        {
            desc.RTVFormats[i] = pRenderPass->getRtvFormat(i);
        }
        desc.DSVFormat = pRenderPass->getDsvFormat();
    }
    desc.BlendState.IndependentBlendEnable = independentBlendEnable;
    desc.BlendState.AlphaToCoverageEnable = false;

    D3D12_INPUT_LAYOUT_DESC& inputDesc = desc.InputLayout;
    inputDesc.NumElements = 0;
    inputDesc.pInputElementDescs = nullptr;
    {
        VertexInputs::D3DVertexInput* layout = VertexInputs::obtain(pipelineState.graphics.inputLayoutId);
        if (layout)
        {
            inputDesc.NumElements = static_cast<U32>(layout->elements.size());
            inputDesc.pInputElementDescs = layout->elements.data();
        }
    }
    
    ID3D12PipelineState* pipeline = nullptr;
    HRESULT result = pDevice->CreateGraphicsPipelineState(&desc, __uuidof(ID3D12PipelineState), (void**)&pipeline);
    R_ASSERT(SUCCEEDED(result));
    return pipeline;
}


R_INTERNAL 
ID3D12PipelineState* createComputePipelineState(U32 nodeMask, ID3D12Device* pDevice, const D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    ID3D12PipelineState* pPipelineState     = nullptr;
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc  = { };
    desc.pRootSignature                     = pipelineState.rootSignature;
    desc.CS.pShaderBytecode                 = program->compute.csBytecode->GetBufferPointer();
    desc.CS.BytecodeLength                  = program->compute.csBytecode->GetBufferSize();
    desc.Flags                              = D3D12_PIPELINE_STATE_FLAG_NONE;
    desc.NodeMask                           = nodeMask;
    HRESULT result = pDevice->CreateComputePipelineState(&desc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
    R_ASSERT(SUCCEEDED(result));
    return pPipelineState;
}


R_INTERNAL 
ID3D12PipelineState* createPipelineState(U32 nodeMask, ID3D12Device* pDevice, D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    ID3D12PipelineState* createdPipelineState = nullptr;
    switch (pipelineState.pipelineType)
    {
        case BindType_Graphics:
            createdPipelineState = createGraphicsPipelineState(nodeMask, pDevice, program, pipelineState);
            break;
        case BindType_RayTrace:
            R_NO_IMPL();
            break;
        case BindType_Compute:
            createdPipelineState = createComputePipelineState(nodeMask, pDevice, program, pipelineState);
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
        desc.Flags = layout.flags; // TODO: None for now, but we might want to try and optimize this?

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
    ID3D12Device* pDevice = pContext->getDevice()->castTo<D3D12Device>()->get();
    auto iter = g_pipelineStateMap.find(pipelineId);
    if (iter == g_pipelineStateMap.end())
    {
        // We didn't find a similar pipeline state, need to create a new one.
        D3D::Cache::D3DShaderProgram* program = D3D::Cache::obtainShaderProgram(pipelineState.shaderProgramId, pipelineState.permutation);
        ID3D12PipelineState* pipeline = createPipelineState(0, pDevice, program, pipelineState);
        g_pipelineStateMap.insert(std::make_pair(pipelineId, pipeline));
        retrievedPipelineState = g_pipelineStateMap[pipelineId];
    }
    else
    {
        retrievedPipelineState = iter->second;
    }
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



void cleanUpPipelines()
{
    for (auto& iter : g_pipelineStateMap)
    {
        iter.second->Release();
    }
    g_pipelineStateMap.clear();
}
} // Pipelines
} // Recluse