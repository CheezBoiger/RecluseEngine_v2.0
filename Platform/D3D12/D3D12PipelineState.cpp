//
#include "D3D12Device.hpp"
#include "D3D12PipelineState.hpp"
#include "D3D12RenderPass.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include "D3D12ShaderCache.hpp"
#include "D3D12RenderPass.hpp"

#include "Recluse/Structures/LifetimeCache.hpp"

#include <map>
#include <unordered_map>
#include <array>

#include <wrl.h>

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


struct PSO
{
    ID3D12PipelineState* pso;
    Hash64 rootSignatureHash;
};

std::map<DeviceId, LifetimeCache<PipelineStateId, PSO>> g_pipelineStateMap;
std::map<DeviceId, std::unordered_map<Hash64, SharedReferenceObject<ID3D12RootSignature*>>> g_rootSignatures;

std::map<DeviceId, std::map<PipelineStateId, ID3D12PipelineState*>>      g_persistentPipelneStateMap;


R_DECLARE_GLOBAL_U32(g_d3d12MaxPipelineAge, 4098, "D3D12.MaxPipelineAge");
R_DECLARE_GLOBAL_BOOLEAN(g_allowPipelineCaching, false, "D3D12.EnablePipelineCache");


namespace VertexInputs {


std::map<DeviceId, std::map<Hash64, ReferenceCounter<D3DVertexInput>>>      g_vertexLayouts;
std::map<DeviceId, std::unordered_map<VertexInputLayoutId, Hash64>>         g_layouts;


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
    auto iter = g_layouts[deviceId].find(id);
    if (iter != g_layouts[deviceId].end())
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

        // Insert the hash.
        Hash64 hh = inputs.hash();
        auto it = g_vertexLayouts[deviceId].find(hh);
        if (it == g_vertexLayouts[deviceId].end())
            g_vertexLayouts[deviceId].insert(std::make_pair(hh, inputs));
        else
            it->second.addReference();

        g_layouts[deviceId].insert(std::make_pair(id, hh));
    }
    return RecluseResult_Ok;
}


D3DVertexInput* obtainLayout(DeviceId deviceId, Hash64 h)
{
    auto it = g_vertexLayouts[deviceId].find(h);
    if (it == g_vertexLayouts[deviceId].end())
        return nullptr;
    return &it->second.get();
}


ResultCode unload(DeviceId deviceId, VertexInputLayoutId id)
{
    auto& iter = g_layouts[deviceId].find(id);
    if (iter != g_layouts[deviceId].end())
    {
        Hash64 h = iter->second;
        g_layouts[deviceId].erase(iter);
        auto vli = g_vertexLayouts[deviceId].find(h);
        if (vli != g_vertexLayouts[deviceId].end())
        {
            if (vli->second.release() == 0)
                g_vertexLayouts[deviceId].erase(vli);
        }
        return RecluseResult_Ok;
    }
    return RecluseResult_NotFound;
}


Bool unloadAll(DeviceId deviceId)
{
    g_vertexLayouts[deviceId].clear();
    g_layouts[deviceId].clear();
    return true;
}


D3DVertexInput* obtain(DeviceId deviceId, VertexInputLayoutId layoutId)
{
    // If we request null argument, then we are essentially clearing.
    if (layoutId == (VertexInputLayoutId)VertexInputLayout::VertexLayout_Null)
        return nullptr;

    auto& iter = g_layouts[deviceId].find(layoutId);
    if (iter != g_layouts[deviceId].end())
    {
        return obtainLayout(deviceId, iter->second);
    }
    else
    {
        R_ASSERT_FORMAT(false, "No Vertex input found for the layoutId(%d)", layoutId);
        return nullptr;
    }
}
} // VertexInputs

typedef struct Direct3DPipelineCacheHeader
{
    U32 headerLength;
    UINT headerVersion;
    UINT deviceId;
    UINT uuid;
} Direct3DPipelineCacheHeader;


Bool cachePipeline(D3D12Device* device, PipelineStateId pipelineId, ID3D12PipelineState* pipelineState)
{
    if (!g_allowPipelineCaching)
        return false;
    if (!pipelineState)
        return false;

    using namespace Microsoft::WRL;
    ComPtr<ID3DBlob> cachedBlob;
    pipelineState->GetCachedBlob(&cachedBlob);
    
    SIZE_T bufferSizeBytes = cachedBlob->GetBufferSize();
    LPVOID ptr = cachedBlob->GetBufferPointer();
    
    return true;
}


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
    desc.NumRenderTargets = pipelineState.state.graphics.numRenderTargets;
    desc.IBStripCutValue = pipelineState.state.graphics.indexStripCut;
    desc.PrimitiveTopologyType = pipelineState.state.graphics.topologyType;
    for (U32 i = 0; i < pipelineState.state.graphics.numRenderTargets; ++i)
    {
        desc.RTVFormats[i] = pipelineState.state.graphics.rtvFormats[i];
    }
    desc.DSVFormat = pipelineState.state.graphics.dsvFormat;

    R_ASSERT_FORMAT(program->graphics.vsBytecode, "Pipeline requires vertex shader to be created!");
    
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
    desc.RasterizerState.AntialiasedLineEnable = pipelineState.state.graphics.antiAliasedLineEnable;
    desc.RasterizerState.FillMode = getFillMode(pipelineState.state.graphics.polygonMode);
    desc.RasterizerState.CullMode = getCullMode(pipelineState.state.graphics.cullMode);
    desc.RasterizerState.DepthClipEnable = pipelineState.state.graphics.depthClampEnable;
    desc.RasterizerState.DepthBias  = pipelineState.state.graphics.depthBiasEnable ? 1 : 0;
    desc.RasterizerState.FrontCounterClockwise = (pipelineState.state.graphics.frontFace == FrontFace_CounterClockwise ? true : false);
    desc.RasterizerState.MultisampleEnable = false;
    desc.RasterizerState.AntialiasedLineEnable = false;
    desc.RasterizerState.SlopeScaledDepthBias = 0.0f;

    desc.DepthStencilState.DepthEnable = pipelineState.state.graphics.depthStencil.depthTestEnable;
    desc.DepthStencilState.StencilEnable = pipelineState.state.graphics.depthStencil.stencilTestEnable;
    desc.DepthStencilState.DepthWriteMask = pipelineState.state.graphics.depthStencil.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    desc.DepthStencilState.DepthFunc = getNativeComparisonFunction(pipelineState.state.graphics.depthStencil.depthCompareOp);
    desc.DepthStencilState.StencilReadMask = pipelineState.state.graphics.depthStencil.stencilReadMask;
    desc.DepthStencilState.StencilWriteMask = pipelineState.state.graphics.depthStencil.stencilWriteMask;
    desc.DepthStencilState.FrontFace = fillStencilState(pipelineState.state.graphics.depthStencil.front);
    desc.DepthStencilState.BackFace = fillStencilState(pipelineState.state.graphics.depthStencil.back);
    Bool independentBlendEnable = false;
    for (U32 i = 0; i < pipelineState.state.graphics.numRenderTargets; ++i)
    {
        D3D12_RENDER_TARGET_BLEND_DESC& rtBlend     = desc.BlendState.RenderTarget[i];
        const RenderTargetBlendState& rtBlendState  = pipelineState.state.graphics.blendState.attachments[i];
        rtBlend.RenderTargetWriteMask               = rtBlendState.colorWriteMask;
        rtBlend.BlendEnable                         = rtBlendState.blendEnable;
        rtBlend.BlendOp                             = getBlendOp(rtBlendState.colorBlendOp);
        rtBlend.BlendOpAlpha                        = getBlendOp(rtBlendState.alphaBlendOp);
        rtBlend.LogicOp                             = getLogicOp(pipelineState.state.graphics.blendState.logicOp);
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
        VertexInputs::D3DVertexInput* layout = VertexInputs::obtain(deviceId, pipelineState.state.graphics.inputLayoutId);
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
ID3D12PipelineState* createMeshGraphicsPipeline(U32 nodeMask, ID3D12Device2* pDevice, const D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState)
{
    R_D3D12_MESH_SHADER_PIPELINE_STATE_DESC desc = { };
    desc.pRootSignature = pipelineState.rootSignature;
    desc.NodeMask = 0;
    desc.NumRenderTargets = pipelineState.state.graphics.numRenderTargets;
    desc.PrimitiveTopologyType = pipelineState.state.graphics.topologyType;
    for (U32 i = 0; i < pipelineState.state.graphics.numRenderTargets; ++i)
    {
        desc.RTVFormats[i] = pipelineState.state.graphics.rtvFormats[i];
    }
    desc.DSVFormat = pipelineState.state.graphics.dsvFormat;

    R_ASSERT_FORMAT(program->graphics.msBytecode, "Mesh shader pipeline requires mesh shader to function!");

    desc.MS.pShaderBytecode = program->graphics.msBytecode->ptr;
    desc.MS.BytecodeLength = program->graphics.msBytecode->sizeBytes;

    if (program->graphics.asBytecode)
    {
        desc.AS.pShaderBytecode = program->graphics.asBytecode->ptr;
        desc.AS.BytecodeLength = program->graphics.asBytecode->sizeBytes;
    }
    
    if (program->graphics.psBytecode)
    {
        desc.PS.pShaderBytecode = program->graphics.psBytecode->ptr;
        desc.PS.BytecodeLength = program->graphics.psBytecode->sizeBytes;
    }

    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    desc.RasterizerState.AntialiasedLineEnable = pipelineState.state.graphics.antiAliasedLineEnable;
    desc.RasterizerState.FillMode = getFillMode(pipelineState.state.graphics.polygonMode);
    desc.RasterizerState.CullMode = getCullMode(pipelineState.state.graphics.cullMode);
    desc.RasterizerState.DepthClipEnable = pipelineState.state.graphics.depthClampEnable;
    desc.RasterizerState.DepthBias  = pipelineState.state.graphics.depthBiasEnable ? 1 : 0;
    desc.RasterizerState.FrontCounterClockwise = (pipelineState.state.graphics.frontFace == FrontFace_CounterClockwise ? true : false);
    desc.RasterizerState.MultisampleEnable = false;
    desc.RasterizerState.AntialiasedLineEnable = false;
    desc.RasterizerState.SlopeScaledDepthBias = 0.0f;

    desc.DepthStencilState.DepthEnable = pipelineState.state.graphics.depthStencil.depthTestEnable;
    desc.DepthStencilState.StencilEnable = pipelineState.state.graphics.depthStencil.stencilTestEnable;
    desc.DepthStencilState.DepthWriteMask = pipelineState.state.graphics.depthStencil.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    desc.DepthStencilState.DepthFunc = getNativeComparisonFunction(pipelineState.state.graphics.depthStencil.depthCompareOp);
    desc.DepthStencilState.StencilReadMask = pipelineState.state.graphics.depthStencil.stencilReadMask;
    desc.DepthStencilState.StencilWriteMask = pipelineState.state.graphics.depthStencil.stencilWriteMask;
    desc.DepthStencilState.FrontFace = fillStencilState(pipelineState.state.graphics.depthStencil.front);
    desc.DepthStencilState.BackFace = fillStencilState(pipelineState.state.graphics.depthStencil.back);
    Bool independentBlendEnable = false;
    for (U32 i = 0; i < pipelineState.state.graphics.numRenderTargets; ++i)
    {
        D3D12_RENDER_TARGET_BLEND_DESC& rtBlend     = desc.BlendState.RenderTarget[i];
        const RenderTargetBlendState& rtBlendState  = pipelineState.state.graphics.blendState.attachments[i];
        rtBlend.RenderTargetWriteMask               = rtBlendState.colorWriteMask;
        rtBlend.BlendEnable                         = rtBlendState.blendEnable;
        rtBlend.BlendOp                             = getBlendOp(rtBlendState.colorBlendOp);
        rtBlend.BlendOpAlpha                        = getBlendOp(rtBlendState.alphaBlendOp);
        rtBlend.LogicOp                             = getLogicOp(pipelineState.state.graphics.blendState.logicOp);
        rtBlend.DestBlend                           = getBlendFactor(rtBlendState.dstColorBlendFactor);
        rtBlend.DestBlendAlpha                      = getBlendFactor(rtBlendState.dstAlphaBlendFactor);
        rtBlend.SrcBlend                            = getBlendFactor(rtBlendState.srcColorBlendFactor);
        rtBlend.SrcBlendAlpha                       = getBlendFactor(rtBlendState.srcAlphaBlendFactor);
        independentBlendEnable |= rtBlend.BlendEnable;
    }

    desc.BlendState.IndependentBlendEnable = independentBlendEnable;
    desc.BlendState.AlphaToCoverageEnable = false;
    
    ID3D12PipelineState* pipeline = nullptr;
    
    RD3D12MeshShaderStreamDescription meshStreamDesc = RD3D12MeshShaderStreamDescription(desc);
    D3D12_PIPELINE_STATE_STREAM_DESC pipelineDesc = { };
    pipelineDesc.pPipelineStateSubobjectStream = &meshStreamDesc;
    pipelineDesc.SizeInBytes = sizeof(meshStreamDesc);
    HRESULT result = pDevice->CreatePipelineState(&pipelineDesc, __uuidof(ID3D12PipelineState), (void**)&pipeline);
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
    pipelineConfig.MaxTraceRecursionDepth = pipelineState.state.raytrace.rayRecursionDepth;
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
ID3D12PipelineState* createPipelineState(U32 nodeMask, DeviceId deviceId, D3D12Device* pDevice, D3D::Cache::D3DShaderProgram* program, const PipelineStateObject& pipelineState, Hash64 rootSignatureHash)
{
    ID3D12PipelineState* createdPipelineState = nullptr;
    switch (pipelineState.pipelineType)
    {
        case BindType_Graphics:
            // A separate pipeline creation function is needed if we plan on creating a pipeline with mesh shaders.
            createdPipelineState = program->graphics.usesMeshShaders 
                ? createMeshGraphicsPipeline(nodeMask, pDevice->get2(), program, pipelineState) 
                : createGraphicsPipelineState(nodeMask, deviceId, pDevice->get(), program, pipelineState);
            break;
        case BindType_RayTrace:
            R_NO_IMPL();
            createdPipelineState = createRaytracingPipeline(nodeMask, pDevice->get(), program, pipelineState);
            break;
        case BindType_Compute:
            createdPipelineState = createComputePipelineState(nodeMask, pDevice->get(), program, pipelineState);
            break;
        default:
            break;        
    }
    R_ASSERT(createdPipelineState != nullptr);

    if (createdPipelineState)
    {
        // Increment the root signature reference for this pipeline state.
        g_rootSignatures[deviceId][rootSignatureHash].add();
    }
    
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

    U32 parameterCount = 0;
    U32 descriptorRangesCount = 0;
    for (u32 set = 0; set < layout.numSets; ++set)
    {
        const Pipelines::RootSigLayout::Set& space = layout.sets[set];
        // One parameter for cbv/srv/uav heaps.
        U32 rdescriptors = (space.cbvCount + space.srvCount + space.uavCount);
        parameterCount += (rdescriptors > 0) ? 1 : 0;
        // Other parameter for sampler heaps.
        parameterCount += space.samplerCount > 0 ? 1 : 0;

        descriptorRangesCount += rdescriptors + space.samplerCount;
    }

    //if (parameterCount == 0)
    //    return pRootSig;

    D3D12_ROOT_SIGNATURE_DESC desc = { };
    // For CbvSrvUavs and Samplers.
    std::vector<D3D12_ROOT_PARAMETER> resourceParameters(parameterCount);
    std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges(descriptorRangesCount);

    U32 rangeOffset = 0;
    U32 paramOffset = 0;

    for (u32 set = 0; set < layout.numSets; ++set)
    {
        const Pipelines::RootSigLayout::Set& space = layout.sets[set];
        const U32 totalDescriptors = space.cbvCount + space.samplerCount + space.srvCount + space.uavCount;

        R_ASSERT(totalDescriptors != 0);
        if (totalDescriptors == 0)
            continue;

        U32 offsetInDescriptors = 0;
        U32 rangeCount = 0;
        U32 prevRangeOffset = rangeOffset;

        if (space.cbvCount > 0)
        {
            D3D12_DESCRIPTOR_RANGE range = { };
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            range.NumDescriptors = space.cbvCount;
            range.BaseShaderRegister = space.baseCbv;
            range.RegisterSpace = set;
            range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
            offsetInDescriptors += space.cbvCount;
            descriptorRanges[rangeOffset++] = range;
            ++rangeCount;
        }
        if (space.srvCount > 0)
        {
            D3D12_DESCRIPTOR_RANGE range = { };
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.NumDescriptors = space.srvCount;
            range.BaseShaderRegister = space.baseSrv;
            range.RegisterSpace = set;
            range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
            offsetInDescriptors += space.srvCount;
            descriptorRanges[rangeOffset++] = range;
            ++rangeCount;
        }
        if (space.uavCount > 0)
        {
            D3D12_DESCRIPTOR_RANGE range = { };
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            range.NumDescriptors = space.uavCount;
            range.BaseShaderRegister = space.baseUav;
            range.RegisterSpace = set;
            range.OffsetInDescriptorsFromTableStart = offsetInDescriptors;
            offsetInDescriptors += space.uavCount;
            descriptorRanges[rangeOffset++] = range;
            ++rangeCount;
        }

        {
            D3D12_ROOT_PARAMETER parameter = { };
            parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            parameter.DescriptorTable.NumDescriptorRanges = rangeCount;
            parameter.DescriptorTable.pDescriptorRanges = &descriptorRanges[prevRangeOffset];
        
            resourceParameters[paramOffset++] = parameter;
        }

        if (space.samplerCount > 0)
        {
            D3D12_DESCRIPTOR_RANGE range = { };
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            range.NumDescriptors = space.samplerCount;
            range.BaseShaderRegister = space.baseSampler;
            range.RegisterSpace = set;
            range.OffsetInDescriptorsFromTableStart = 0; // Doesn't need to have an offset, since samplers will be in their own table.
            U32 samplerRangeOffset = rangeOffset;
            descriptorRanges[rangeOffset++] = range;

            D3D12_ROOT_PARAMETER sparameter = { };
            sparameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            sparameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            sparameter.DescriptorTable.NumDescriptorRanges = 1;
            sparameter.DescriptorTable.pDescriptorRanges = &descriptorRanges[samplerRangeOffset];
            resourceParameters[paramOffset++] = sparameter;
        }
    }

    desc.NumParameters = resourceParameters.size();
    desc.NumStaticSamplers = 0;
    desc.pParameters = resourceParameters.data();
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
    // TODO: This function is supposed to create separate root parameter structs, that define different layouts of the 
    //          shader registers. We shouldn't really allow separate parameter registers, but it would allow a different option to 
    //          create layouts for shader data.
    R_NO_IMPL();
    return nullptr;
}

ID3D12PipelineState* makePipelineState(D3D12Context* pContext, const PipelineStateObject& pipelineState, Hash64 rootSignatureHash)
{
    ID3D12PipelineState* retrievedPipelineState = nullptr;
    PipelineStateId pipelineId = serializePipelineState(pipelineState);
    D3D12Device* device = pContext->getDevice()->castTo<D3D12Device>();
    DeviceId deviceId = device->getDeviceId();
    if (!g_pipelineStateMap[deviceId].inCache(pipelineId))
    {
        // We didn't find a similar pipeline state, need to create a new one.
        D3D::Cache::D3DShaderProgram* program = D3D::Cache::obtainShaderProgram(pipelineState.shaderProgramId, pipelineState.permutation);
        ID3D12PipelineState* pipeline = createPipelineState(0, deviceId, device, program, pipelineState, rootSignatureHash);
        PSO pso = { pipeline, rootSignatureHash };
        retrievedPipelineState = g_pipelineStateMap[deviceId].insert(pipelineId, std::move(pso))->pso;
    }
    else
    {
        retrievedPipelineState = g_pipelineStateMap[deviceId].refer(pipelineId)->pso;
    }
    return retrievedPipelineState;
}


ID3D12RootSignature* makeRootSignature(D3D12Device* pDevice, const RootSigLayout& layout)
{
    ID3D12RootSignature* rootSignature = nullptr;
    Hash64 hash = layout.hash0;
    DeviceId deviceId = pDevice->getDeviceId();
    if (g_rootSignatures[deviceId].find(hash) == g_rootSignatures[deviceId].end())
    {
        rootSignature = internalCreateRootSignatureWithTable(pDevice->get(), layout);
        g_rootSignatures[deviceId].insert(std::make_pair(hash, std::move(rootSignature)));
        // TODO: Figure out how to internally create and own, instead of having the command list do this.
        g_rootSignatures[deviceId][hash].release(); // release initially.
    }
    else
    {
        rootSignature = *g_rootSignatures[deviceId][hash];
    }
    return rootSignature;
}


CpuDescriptorTable makeDescriptorSrvCbvUavTable(D3D12Device* pDevice, U32 space, const RootSigLayout& layout, const RootSigResourceTable& resourceTable)
{
    DescriptorHeapAllocationManager* pManager = pDevice->getDescriptorHeapManager();

    const Pipelines::RootSigLayout::Set& set = layout.sets[space];
    const U32 descriptorCount = set.cbvCount + set.srvCount + set.uavCount;

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles(descriptorCount);
    U32 i = 0;

    const Pipelines::RootSigResourceTable::Set& resources = resourceTable.sets[space];

    for (U32 j = 0; j < set.cbvCount; ++j)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = resources.cbvs[j];
        if (handle.ptr != DescriptorTable::invalidCpuAddress.ptr)
            handles[i++] = resources.cbvs[j];
        else
            handles[i++] = pManager->nullCbvDescriptor();
    }
    for (U32 j = 0; j < set.srvCount; ++j)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = resources.srvs[j];
        if (handle.ptr != DescriptorTable::invalidCpuAddress.ptr)
            handles[i++] = resources.srvs[j];
        else
            handles[i++] = pManager->nullSrvDescriptor();
    }
    for (U32 j = 0; j < set.uavCount; ++j)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = resources.uavs[j];
        if (handle.ptr != DescriptorTable::invalidCpuAddress.ptr)
            handles[i++] = resources.uavs[j];
        else
            handles[i++] = pManager->nullUavDescriptor();
    }

    Hash64 hash = recluseHashFast(handles.data(), sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * handles.size());
    DeviceId deviceId = pDevice->getDeviceId();
    
    // We are essentially searching if there are already the same batched descriptor tables. We don't want to create duplicates,
    // so we refer to already allocated tables if the current bind is the same. Otherwise, we copy a new descriptors table and store for 
    // the current frame. These get cleared out every frame.
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


CpuDescriptorTable              makeDescriptorSamplertable(D3D12Device* pDevice, U32 space, const RootSigLayout& layout, const RootSigResourceTable& resourceTable)
{
    DescriptorHeapAllocationManager* pManager = pDevice->getDescriptorHeapManager();

    const U32 descriptorCount = layout.sets[space].samplerCount;

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles(descriptorCount);
    U32 offset = 0; 

    const Pipelines::RootSigResourceTable::Set& set = resourceTable.sets[space];
    for (U32 i = 0; i < layout.sets[space].samplerCount; ++ i)
    {
        handles[offset++] = set.samplers[i];
    }

    Hash64 hash = recluseHashFast(handles.data(), sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * descriptorCount);
    DeviceId deviceId = pDevice->getDeviceId();
    auto iter = m_cachedSamplerTables[deviceId].find(hash);
    if (iter == m_cachedSamplerTables[deviceId].end())
    {
        CpuDescriptorTable table = pManager->copyDescriptorsToTable(CpuHeapType_Sampler, handles.data(), descriptorCount);
        m_cachedSamplerTables[deviceId].insert(std::make_pair(hash, table));
        return table; 
    }
    else
    {
        return iter->second;
    }
}


void destroyRootSignature(DeviceId deviceId, Hash64 rootSignatureHash)
{
    auto& it = g_rootSignatures[deviceId].find(rootSignatureHash);
    if (it != g_rootSignatures[deviceId].end())
    {
        if (it->second.release() == 0)
        {
            R_DEBUG(R_CHANNEL_D3D12, "Pipeline destruction. Destroying root signature.");
            (*it->second)->Release();
            g_rootSignatures[deviceId].erase(rootSignatureHash);
        }
    }
}


void cleanUpRootSigs(DeviceId deviceId)
{

    //g_rootSignatures[deviceId].forEach(
    //    [] (Hash64, ID3D12RootSignature* rootSignature) -> void 
    //    {
    //        R_DEBUG(R_CHANNEL_D3D12, "Destroying root signature.");
    //        rootSignature->Release();
    //    }
    //);

    for (auto& it : g_rootSignatures[deviceId])
    {
        R_DEBUG(R_CHANNEL_D3D12, "Destroying root signature");
        (*it.second)->Release();
    }
    
    g_rootSignatures[deviceId].clear();
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
        [&] (PipelineStateId, PSO& pipelineState) -> void
        {
            R_DEBUG(R_CHANNEL_D3D12, "Destroying pipeline.");
            pipelineState.pso->Release();
            destroyRootSignature(deviceId, pipelineState.rootSignatureHash);
        });
    g_pipelineStateMap[deviceId].clear();
}


void updateT(D3D12Device* pDevice)
{
    g_pipelineStateMap[pDevice->getDeviceId()].updateTick();
    //g_rootSignatures[pDevice->getDeviceId()].updateTick();
}


void checkPipelines(D3D12Device* pDevice)
{
    g_pipelineStateMap[pDevice->getDeviceId()].check(1, g_d3d12MaxPipelineAge, [&] (PipelineStateId, PSO& pipelineState) -> void
        {
            R_DEBUG(R_CHANNEL_D3D12, "Destroying pipeline.");
            pipelineState.pso->Release();
            destroyRootSignature(pDevice->getDeviceId(), pipelineState.rootSignatureHash);
        });

    //g_rootSignatures[pDevice->getDeviceId()].check(1, g_d3d12MaxPipelineAge, [] (Hash64, ID3D12RootSignature* rootSignature) -> void 
    //    {
    //        R_DEBUG(R_CHANNEL_D3D12, "Destroying root signature");
    //        rootSignature->Release();
    //    });
}


void RootSigLayout::makeHash()
{
    hash0 = 0;
    for (U32 i = 0; i < numSets; ++i)
    {
        hash0 = combineHash64(hash0, recluseHashFast(&sets[i], sizeof(Set)));
    }
    hash0 = combineHash64(hash0, shaderVisibility);
    hash0 = combineHash64(hash0, flags);
    hash0 = combineHash64(hash0, numSets);
}
} // Pipelines
} // D3D12 
} // Recluse