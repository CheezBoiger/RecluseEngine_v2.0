//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"

namespace Recluse {

class Shader;

// Layout bind range is the descriptor layout.
struct LayoutBindRange
{
    // t# is used for srvs
    // u# is used for uavs
    // b# is used for cbuffers.
    DescriptorBindType  bindType;
    U32                 space;
    U32                 range;
};

typedef void* BindLayout;

struct StencilOpState
{
    StencilOp   failOp;
    StencilOp   passOp;
    StencilOp   depthFailOp;
    CompareOp   compareOp;
};


enum InputRate 
{
    InputRate_PerVertex,
    InputRate_PerInstance
};


enum PrimitiveTopology 
{
    PrimitiveTopology_TriangleList,
    PrimitiveTopology_TriangleStrip,
    PrimitiveTopology_PointList,
    PrimitiveTopology_LineList,
    PrimitiveTopology_LineStrip
};


enum FrontFace 
{
    FrontFace_CounterClockwise,
    FrontFace_Clockwise
};

enum PolygonMode 
{
    PolygonMode_Fill,
    PolygonMode_Line,
    PolygonMode_Point
};


enum CullMode 
{
    CullMode_None,
    CullMode_Front,
    CullMode_Back,
    CullMode_FrontAndBack
};


struct VertexAttribute 
{
    enum 
    { 
        // Append the offset for the given attibute based on the previous attribute.
        OffsetAppend = 0xffffffff 
    };
    // Location/input slot that defines the vertex attribute in the vertex shader.
    union {
        U32             location;
        U32             slot;
    };
    U32             offsetBytes;             // offset within the vertex attribute.
    ResourceFormat  format;

    // Semantics are more geared towards hlsl, which will need to be used.
    // Ensure that you provide the proper semantic conventions.
    Semantic        semantic;
    U32             semanticIndex;
    
};


// VertexBinding identifies one vertex data unit, within a vertex buffer.
// Each VertexBinding identifies a vertex buffer each.
struct VertexBinding 
{
    // Binding index, which is maxed between 0, to (VertexInputLayout_BindingCount - 1)
    U32                 binding;
    U32                 stride;                         // Data Step rate between consecutive elements, in bytes. If 0, will use tight packing of vertex.
    InputRate           inputRate;
    VertexAttribute*    pVertexAttributes;
    U32                 numVertexAttributes;
};


enum LogicOp 
{
    LogicOp_Clear,
    LogicOp_And,
    LogicOp_AndReverse,
    LogicOp_Copy,
    LogicOp_AndInverted,
    LogicOp_NoOp,
    LogicOp_Xor,
    LogicOp_Or,
    LogicOp_Nor,
    LogicOp_Equivalent,
    LogicOp_Invert,
    LogicOp_OrReverse,
    LogicOp_CopyInverted,
    LogicOp_OrInverted,
    LogicOp_Nand,
    LogicOp_Set
};

enum BlendFactor 
{
    BlendFactor_Zero,
    BlendFactor_One,
    BlendFactor_SourceColor,
    BlendFactor_OneMinusSourceColor,
    BlendFactor_DestinationColor,
    BlendFactor_OneMinusDestinationColor,
    BlendFactor_SourceAlpha,
    BlendFactor_OneMinusSourceAlpha,
    BlendFactor_DestinationAlpha,
    BlendFactor_OneMinusDestinationAlpha,
    BlendFactor_ConstantColor,
    BlendFactor_OneMinusConstantColor,
    BlendFactor_ConstantAlpha,
    BlendFactor_OneMinusConstantAlpha,
    BlendFactor_SourceAlphaSaturate,
    BlendFactor_SourceOneColor,
    BlendFactor_OneMinusSourceOneColor,
    BlendFactor_SourceOneAlpha,
    BlendFactor_OneMinusSourceOneAlpha
};


enum BlendOp 
{
    BlendOp_Add,
    BlendOp_Subtract,
    BlendOp_ReverseSubtract,
    BlendOp_Min,
    BlendOp_Max    
};


enum ColorComponent 
{
    Color_None = 0,
    Color_R = 0x1,
    Color_G = 0x2, 
    Color_B = 0x4,
    Color_A = 0x8,
    Color_Rgba = (Color_R | Color_G | Color_B | Color_A)
};

typedef U32 ColorComponentMaskFlags;


struct RenderTargetBlendState 
{
    B32                     blendEnable;
    BlendFactor             srcColorBlendFactor;
    BlendFactor             dstColorBlendFactor;
    BlendOp                 colorBlendOp;
    BlendFactor             srcAlphaBlendFactor;
    BlendFactor             dstAlphaBlendFactor;
    BlendOp                 alphaBlendOp;
   ColorComponentMaskFlags  colorWriteMask;
};

struct VertexInputLayout 
{
    enum { VertexInputLayout_BindingCount = 16 };
    enum { VertexLayout_Null = 0xffffffff };
    VertexBinding   vertexBindings[VertexInputLayout_BindingCount];
    U32             numVertexBindings;
};

typedef uint VertexInputLayoutId;

struct DepthStencil 
{
    B8              depthBoundsTestEnable;
    B8              depthTestEnable;
    B8              stencilTestEnable;
    B8              depthWriteEnable;
    U8              stencilReadMask;
    U8              stencilWriteMask;
    U8              stencilReference;
    F32             minDepthBounds;
    F32             maxDepthBounds;
    CompareOp       depthCompareOp;
    StencilOpState  front;
    StencilOpState  back;
};

struct RasterState 
{
    CullMode    cullMode;
    FrontFace   frontFace;
    PolygonMode polygonMode;
    F32         lineWidth = 1.0f;
    F32         depthBiasClamp;
    F32         depthBiasConstantFactor;
    F32         depthBiasSlopFactor;
    B32         depthClampEnable : 1, 
                depthBiasEnable : 1;
};

struct BlendState 
{
    B32     logicOpEnable;
    LogicOp logicOp;
    F32     blendConstants[4];
    RenderTargetBlendState attachments[8];
};

struct TessellationState 
{
    U32 numControlPoints;
};


class VertexInputStore : public Serializable
{
public:
    VertexInputStore(VertexInputLayout* layout)
        : layout(layout) { }

    ResultCode serialize(Archive* archive) const override;
    ResultCode deserialize(Archive* archive) override;

private:
    VertexInputLayout* layout;
};

struct ShaderBytecode
{
    const void* ptr;
    const char* entry;
    U64         sizeBytes;

    ShaderBytecode(const void* bytecode = nullptr, U64 szBytes = 0ull, const char* entry = nullptr)
        : ptr(bytecode)
        , entry(entry)
        , sizeBytes(szBytes) { }
};


class PipelineStateDescription
{
public:
    PipelineStateDescription(BindType bindType) 
        : bindType(bindType)
        , layout(nullptr) { }

    BindLayout          layout;
    U32                 layoutSize;

    BindType            getBindType() const { return bindType; }

private:
    BindType            bindType;
};


class RasterPipelineStateDescription : public PipelineStateDescription
{
public:
    enum GeometryPipeline
    {
        GeometryPipeline_Unknown,
        GeometryPipeline_Traditional,
        GeometryPipeline_Mesh
    };

    PrimitiveTopology   primitiveTopology;
    DepthStencil        depthStencil;
    BlendState          blendState;
    RasterState         rasterState;
    TessellationState   tessellationState;

    ShaderBytecode      ps;

    Bool usesMeshShader() const { return isMeshShader; }

    RasterPipelineStateDescription() = delete;

protected:

    RasterPipelineStateDescription(GeometryPipeline geometryPipelne, Bool isMeshShader = false)
        : PipelineStateDescription(BindType_Graphics)
        , isMeshShader(isMeshShader)
        , primitiveTopology(PrimitiveTopology_PointList)
        , depthStencil({})
        , blendState({})
        , rasterState({})
        , tessellationState({})
        , ps({})
        , geometryPipeline(geometryPipeline)
    { }

private:
    Bool                isMeshShader;
    GeometryPipeline    geometryPipeline;
};


class GraphicsPipelineStateDescription : public RasterPipelineStateDescription
{
public:
    GraphicsPipelineStateDescription() 
        : RasterPipelineStateDescription(RasterPipelineStateDescription::GeometryPipeline_Traditional, false)
        , inputLayout(nullptr)
        , vs({})
        , ds({})
        , hs({})
        , gs({})
    { }

    VertexInputLayout*  inputLayout;
    
    ShaderBytecode      vs;
    ShaderBytecode      ds;
    ShaderBytecode      hs;
    ShaderBytecode      gs;
};


class MeshPipelineStateDescription : public RasterPipelineStateDescription
{
public:
    MeshPipelineStateDescription()
        : RasterPipelineStateDescription(RasterPipelineStateDescription::GeometryPipeline_Mesh, true)
        , ms({})
        , as({})
    { }

    ShaderBytecode      ms;
    ShaderBytecode      as;
};


class ComputePipelineStateDescription : public PipelineStateDescription
{
    ComputePipelineStateDescription() 
        : PipelineStateDescription(BindType_Compute)
        , cs({}) { }
    ShaderBytecode cs;

    ComputePipelineStateDescription& setCs(const void* ptr, U32 shaderSizeBytes)
        {
            cs.ptr = ptr;
            cs.sizeBytes = shaderSizeBytes; 
            return (*this); 
        }
};


class RayTracePipelineStateDescription : public PipelineStateDescription
{
    RayTracePipelineStateDescription() 
        : PipelineStateDescription(BindType_RayTrace) { }
};

typedef Hash64 PipelineState;
} // Recluse