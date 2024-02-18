//
#include "D3D12Device.hpp"
#include "D3D12PipelineState.hpp"
#include "D3D12RenderPass.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include "D3D12ShaderCache.hpp"
#include "D3D12RenderPass.hpp"

#include "Graphics/LifetimeCache.hpp"

#include <map>
#include <unordered_map>
#include <array>

namespace Recluse {
namespace D3D12 {

std::map<Semantic, const char*> g_semanticMap = {
    { Semantic_Position, "POSITION" },
    { Semantic_Normal, "NORMAL" },
    { Semantic_Texcoord, "TEXCOORD" },
    { Semantic_Tangent, "TANGENT" },
    { Semantic_Binormal, "BINORMAL" },
    { Semantic_Color, "COLOR" },
    { Semantic_TessFactor, "TESSFACTOR" }
};


std::map<DeviceId,  std::unordered_map<Hash64, CpuDescriptorTable>> m_cachedCpuDescriptorTables;
std::map<DeviceId,  std::unordered_map<Hash64, CpuDescriptorTable>> m_cachedSamplerTables;


namespace Pipelines {


std::map<DeviceId, LifetimeCache<PipelineStateId, ID3D12PipelineState*>> g_pipelineStateMap;
std::map<DeviceId, std::unordered_map<Hash64, ID3D12RootSignature*>> m_rootSignatures;


R_DECLARE_GLOBAL_U32(g_d3d12MaxPipelineAge, 256, "D3D12.MaxPipelineAge");


namespace VertexInputs {


std::map<DeviceId, std::unordered_map<VertexInputLayoutId, D3DVertexInput>> g_vertexLayouts;


D3D12_INPUT_CLASSIFICATION getInputClassification(InputRate inputRate)
{
    switch (inputRate)
    {
        case InputRate_PerInstance: return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        default:
        case InputRate_PerVertex:   return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    }
}


ResultCode make(DeviceId deviceId, VertexInputLayoutId id, const VertexInputLayout& layout)
{
    R_ASSERT(layout.numVertexBindings < VertexInputLayout::VertexInputLayout_BindingCount);
    auto iter = g_vertexLayouts[deviceId].find(id);
    if (iter != g_vertexLayouts[deviceId].end())
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

        g_vertexLayouts[deviceId].insert(std::make_pair(id, inputs));
    }
    return RecluseResult_Ok;
}


ResultCode unload(DeviceId deviceId, VertexInputLayoutId id)
{
    auto& iter = g_vertexLayouts[deviceId].find(id);
    if (iter != g_vertexLayouts[deviceId].end())
    {
        g_vertexLayouts[deviceId].erase(iter);
        return RecluseResult_Ok;
    }
    return RecluseResult_NotFound;
}


Bool unloadAll(DeviceId deviceId)
{
    g_vertexLayouts[deviceId].clear();
    return true;
}


D3DVertexInput* obtain(DeviceId deviceId, VertexInputLayoutId layoutId)
{
    // If we request null argument, then we are essentially clearing.
    if (layoutId == VertexInputLayout::VertexLayout_Null)
        return nullptr;

    auto& iter = g_vertexLayouts[deviceId].find(layoutId);
    if (iter != g_vertexLayouts[deviceId].end())
    {
        return &iter->second;
    }
    else
    {
        R_ASSERT_FORMAT(false, "No Vertex input found for the layoutId(%d)", layoutId);
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
ID3D12PipelineState* createGraphicsPipelineState(U32 nodeMask, DeviceId deviceId, ID3D12Device* pDevice, const D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = { };
    desc.pRootSignature = pipelineState.rootSignature;
    desc.NodeMask = 0;
    desc.NumRenderTargets = pipelineState.graphics.numRenderTargets;
    desc.IBStripCutValue = pipelineState.graphics.indexStripCut;
    desc.PrimitiveTopologyType = pipelineState.graphics.topologyType;
    for (U32 i = 0; i < pipelineState.graphics.numRenderTargets; ++i)
    {
        desc.RTVFormats[i] = pipelineState.graphics.rtvFormats[i];
    }
    desc.DSVFormat = pipelineState.graphics.dsvFormat;

    R_ASSERT(program->graphics.vsBytecode);
    
    desc.VS.pShaderBytecode = program->graphics.vsBytecode->ptr;
    desc.VS.BytecodeLength = program->graphics.vsBytecode->sizeBytes;
    
    if (program->graphics.psBytecode)
    {
        desc.PS.pShaderBytecode = program->graphics.psBytecode->ptr;
        desc.PS.BytecodeLength = program->graphics.psBytecode->sizeBytes;
    }

    if (program->graphics.hsBytecode)
    {
        desc.HS.pShaderBytecode = program->graphics.hsBytecode->ptr;
        desc.HS.BytecodeLength = program->graphics.hsBytecode->sizeBytes;
    }

    if (program->graphics.dsBytecode)
    {
        desc.DS.pShaderBytecode = program->graphics.dsBytecode->ptr;
        desc.DS.BytecodeLength = program->graphics.dsBytecode->sizeBytes;
    }

    if (program->graphics.gsBytecode)
    {
        desc.GS.pShaderBytecode = program->graphics.gsBytecode->ptr;
        desc.GS.BytecodeLength = program->graphics.gsBytecode->sizeBytes;
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

    desc.BlendState.IndependentBlendEnable = independentBlendEnable;
    desc.BlendState.AlphaToCoverageEnable = false;

    D3D12_INPUT_LAYOUT_DESC& inputDesc = desc.InputLayout;
    inputDesc.NumElements = 0;
    inputDesc.pInputElementDescs = nullptr;
    {
        VertexInputs::D3DVertexInput* layout = VertexInputs::obtain(deviceId, pipelineState.graphics.inputLayoutId);
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
ID3D12PipelineState* createMeshGraphicsPipeline(U32 nodeMask, ID3D12Device* pDevice, const D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    R_D3D12_MESH_SHADER_PIPELINE_STATE_DESC desc = { };
    desc.pRootSignature = pipelineState.rootSignature;
    desc.NodeMask = 0;
    desc.NumRenderTargets = pipelineState.graphics.numRenderTargets;
    desc.PrimitiveTopologyType = pipelineState.graphics.topologyType;
    for (U32 i = 0; i < pipelineState.graphics.numRenderTargets; ++i)
    {
        desc.RTVFormats[i] = pipelineState.graphics.rtvFormats[i];
    }
    desc.DSVFormat = pipelineState.graphics.dsvFormat;

    R_ASSERT(program->graphics.vsBytecode);
    
    if (program->graphics.psBytecode)
    {
        desc.PS.pShaderBytecode = program->graphics.psBytecode->ptr;
        desc.PS.BytecodeLength = program->graphics.psBytecode->sizeBytes;
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

    desc.BlendState.IndependentBlendEnable = independentBlendEnable;
    desc.BlendState.AlphaToCoverageEnable = false;
    
    ID3D12PipelineState* pipeline = nullptr;
    ID3D12Device2* pDevice2 = nullptr;
    HRESULT result = pDevice->QueryInterface<ID3D12Device2>(&pDevice2);
    if (SUCCEEDED(result))
    {
        RD3D12MeshShaderStreamDescription meshStreamDesc = RD3D12MeshShaderStreamDescription(desc);
        D3D12_PIPELINE_STATE_STREAM_DESC pipelineDesc = { };
        pipelineDesc.pPipelineStateSubobjectStream = &meshStreamDesc;
        pipelineDesc.SizeInBytes = sizeof(meshStreamDesc);
        result = pDevice2->CreatePipelineState(&pipelineDesc, __uuidof(ID3D12PipelineState), (void**)&pipeline);
        pDevice2->Release();
    }
    R_ASSERT(SUCCEEDED(result));
    return pipeline;
}


R_INTERNAL 
ID3D12PipelineState* createComputePipelineState(U32 nodeMask, ID3D12Device* pDevice, const D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    ID3D12PipelineState* pPipelineState     = nullptr;
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc  = { };
    desc.pRootSignature                     = pipelineState.rootSignature;
    desc.CS.pShaderBytecode                 = program->compute.csBytecode->ptr;
    desc.CS.BytecodeLength                  = program->compute.csBytecode->sizeBytes;
    desc.Flags                              = D3D12_PIPELINE_STATE_FLAG_NONE;
    desc.NodeMask                           = nodeMask;
    HRESULT result = pDevice->CreateComputePipelineState(&desc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
    R_ASSERT(SUCCEEDED(result));
    return pPipelineState;
}


R_INTERNAL 
ID3D12PipelineState* createRaytracingPipeline(U32 nodeMask, ID3D12Device* pDevice, const D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    ID3D12PipelineState* pipeline = nullptr;
    ID3D12Device5* pDevice5 = nullptr;
    pDevice->QueryInterface<ID3D12Device5>(&pDevice5);

    D3D12_STATE_OBJECT_DESC pipelineDesc = { };
    pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    std::vector<D3D12_STATE_SUBOBJECT> subobjects;
 
    // DXIL Library shader setup.
    D3D12_STATE_SUBOBJECT lib;
    lib.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    D3D12_STATE_SUBOBJECT subob;
    subob.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    lib.pDesc = nullptr;
    subobjects.push_back(lib);
    
    // Pipeline config.
    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = { };
    pipelineConfig.MaxTraceRecursionDepth = pipelineState.raytrace.rayRecursionDepth;
    D3D12_STATE_SUBOBJECT pipeConfigObj = {};
    pipeConfigObj.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    pipeConfigObj.pDesc = &pipelineConfig;
    subobjects.push_back(pipeConfigObj);

    HRESULT result = pDevice5->CreateStateObject(&pipelineDesc, __uuidof(ID3D12PipelineState), (void**)&pipeline);
    R_ASSERT(result == S_OK);
    pDevice5->Release();    
    return pipeline;
}


R_INTERNAL 
ID3D12PipelineState* createPipelineState(U32 nodeMask, DeviceId deviceId, ID3D12Device* pDevice, D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    ID3D12PipelineState* createdPipelineState = nullptr;
    switch (pipelineState.pipelineType)
    {
        case BindType_Graphics:
            // A separate pipeline creation function is needed if we plan on creating a pipeline with mesh shaders.
            createdPipelineState = program->graphics.usesMeshShaders 
                ? createMeshGraphicsPipeline(nodeMask, pDevice, program, pipelineState) 
                : createGraphicsPipelineState(nodeMask, deviceId, pDevice, program, pipelineState);
            break;
        case BindType_RayTrace:
            R_NO_IMPL();
            createdPipelineState = createRaytracingPipeline(nodeMask, pDevice, program, pipelineState);
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

    D3D12_ROOT_SIGNATURE_DESC desc = { };
    // For CbvSrvUavs and Samplers.
    D3D12_ROOT_PARAMETER tableParameters[2] = { };
    std::array<D3D12_DESCRIPTOR_RANGE, 4> ranges = { };
    const Bool hasSamplers = (layout.samplerCount > 0);
    U32 cbvSrvUavRangeIdx = 0;
    U32 offsetInDescriptors = 0;
    if (layout.cbvCount > 0)
    {
        D3D12_DESCRIPTOR_RANGE& range = ranges[cbvSrvUavRangeIdx++];
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range.NumDescriptors = layout.cbvCount;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
        offsetInDescriptors += layout.cbvCount;
    }
    if (layout.srvCount > 0)
    {
        D3D12_DESCRIPTOR_RANGE& range = ranges[cbvSrvUavRangeIdx++];
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors = layout.srvCount;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
        offsetInDescriptors += layout.srvCount;
    }
    if (layout.uavCount > 0)
    {
        D3D12_DESCRIPTOR_RANGE& range = ranges[cbvSrvUavRangeIdx++];
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        range.NumDescriptors = layout.uavCount;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
        offsetInDescriptors += layout.uavCount;
    }
    if (layout.samplerCount > 0)
    {
        D3D12_DESCRIPTOR_RANGE& range = ranges[cbvSrvUavRangeIdx];
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        range.NumDescriptors = layout.samplerCount;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = 0; // Doesn't need to have an offset, since samplers will be in their own table.
        offsetInDescriptors += layout.samplerCount;
    }

    tableParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    tableParameters[0].DescriptorTable.NumDescriptorRanges = cbvSrvUavRangeIdx;
    tableParameters[0].DescriptorTable.pDescriptorRanges = ranges.data();
    tableParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    if (hasSamplers)
    {    
        tableParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        tableParameters[1].DescriptorTable.NumDescriptorRanges = 1;
        tableParameters[1].DescriptorTable.pDescriptorRanges = &ranges[Math::clamp(cbvSrvUavRangeIdx, (U32)0, (U32)ranges.size())];
        tableParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    U32 numParameters = 0;
    if (totalDescriptors)
    {
        numParameters += 1;
    }
    if (hasSamplers)
    {
        numParameters += 1;
    }
    desc.NumParameters = numParameters;
    desc.NumStaticSamplers = 0;
    desc.pParameters = tableParameters;
    desc.pStaticSamplers = nullptr;
    desc.Flags = layout.flags; // TODO: None for now, but we might want to try and optimize this?

    ID3DBlob* pSignature = nullptr;
    ID3DBlob* pErrorBlob = nullptr;
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
    if (pSignature)
    {
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
    DeviceId deviceId = pContext->getDevice()->castTo<D3D12Device>()->getDeviceId();
    if (!g_pipelineStateMap[deviceId].inCache(pipelineId))
    {
        // We didn't find a similar pipeline state, need to create a new one.
        D3D::Cache::D3DShaderProgram* program = D3D::Cache::obtainShaderProgram(pipelineState.shaderProgramId, pipelineState.permutation);
        ID3D12PipelineState* pipeline = createPipelineState(0, deviceId, pDevice, program, pipelineState);
        retrievedPipelineState = *g_pipelineStateMap[deviceId].insert(pipelineId, std::move(pipeline));
    }
    else
    {
        retrievedPipelineState = *g_pipelineStateMap[deviceId].refer(pipelineId);
    }
    return retrievedPipelineState;
}


ID3D12RootSignature* makeRootSignature(D3D12Device* pDevice, const RootSigLayout& layout)
{
    ID3D12RootSignature* rootSignature = nullptr;
    Hash64 hash = recluseHashFast(&layout, sizeof(RootSigLayout));
    DeviceId deviceId = pDevice->getDeviceId();
    auto iter = m_rootSignatures[deviceId].find(hash);
    if (iter == m_rootSignatures[deviceId].end())
    {
        rootSignature = internalCreateRootSignatureWithTable(pDevice->get(), layout);
        m_rootSignatures[deviceId].insert(std::make_pair(hash, rootSignature));
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
    DeviceId deviceId = pDevice->getDeviceId();
    auto iter = m_cachedCpuDescriptorTables[deviceId].find(hash);
    if (iter == m_cachedCpuDescriptorTables[deviceId].end())
    {
        CpuDescriptorTable table = pManager->copyDescriptorsToTable(CpuHeapType_CbvSrvUav, handles.data(), descriptorCount);
        m_cachedCpuDescriptorTables[deviceId].insert(std::make_pair(hash, table));
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
    DeviceId deviceId = pDevice->getDeviceId();
    auto iter = m_cachedSamplerTables[deviceId].find(hash);
    if (iter == m_cachedSamplerTables[deviceId].end())
    {
        CpuDescriptorTable table = pManager->copyDescriptorsToTable(CpuHeapType_Sampler, resourceTable.samplers, descriptorCount);
        m_cachedSamplerTables[deviceId].insert(std::make_pair(hash, table));
        return table; 
    }
    else
    {
        return iter->second;
    }
}


void cleanUpRootSigs(DeviceId deviceId)
{
    for (auto rootsig : m_rootSignatures[deviceId])
    {
        rootsig.second->Release();
    }

    m_rootSignatures[deviceId].clear();
}


void resetTableHeaps(D3D12Device* pDevice)
{
    DescriptorHeapAllocationManager* pManager = pDevice->getDescriptorHeapManager();
    pManager->resetCpuTableHeaps();
    DeviceId deviceId = pDevice->getDeviceId();
    m_cachedCpuDescriptorTables[deviceId].clear();
    m_cachedSamplerTables[deviceId].clear();
}



void cleanUpPipelines(DeviceId deviceId)
{
    g_pipelineStateMap[deviceId].forEach(
        [] (PipelineStateId, ID3D12PipelineState* pipelineState) -> void
        {
            R_DEBUG(R_CHANNEL_D3D12, "Destroying pipeline.");
            pipelineState->Release();
        });
    g_pipelineStateMap[deviceId].clear();
}


void updateT(D3D12Device* pDevice)
{
    g_pipelineStateMap[pDevice->getDeviceId()].updateTick();
}


void checkPipelines(D3D12Device* pDevice)
{
    g_pipelineStateMap[pDevice->getDeviceId()].check(g_d3d12MaxPipelineAge, [] (PipelineStateId, ID3D12PipelineState* pipelineState) -> void
        {
            R_DEBUG(R_CHANNEL_D3D12, "Destroying pipeline.");
            pipelineState->Release();
        });
}
} // Pipelines
} // D3D12 
} // Recluse