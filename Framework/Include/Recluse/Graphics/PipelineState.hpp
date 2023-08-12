//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"

namespace Recluse {

class Shader;


struct ShaderModule 
{
    void* byteCode;
    U32 szBytes;
};


struct StencilOpState
{
    StencilOp   failOp;
    StencilOp   passOp;
    StencilOp   depthFailOp;
    CompareOp   compareOp;
    U32         readMask;
    U32         writeMask;
    U32         reference;
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


struct VertexBinding 
{
    U32                 binding;
    U32                 stride;                         // Data Step rate between consecutive elements.
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
    enum { VertexInputLayout_Count = 16 };
    VertexBinding   vertexBindings[VertexInputLayout_Count];
    U32             numVertexBindings;
};

typedef Hash64 VertexInputLayoutId;

struct DepthStencil 
{
    B8  depthBoundsTestEnable;
    B8  depthTestEnable;
    B8  stencilTestEnable;
    B8  depthWriteEnable;
    F32 minDepthBounds;
    F32 maxDepthBounds;
    CompareOp depthCompareOp;
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


namespace Pipeline {

typedef Hash64 PipelineId;
typedef Hash64 RasterId;
typedef Hash64 BlendId;
typedef Hash64 TessellationId;
typedef Hash64 DepthStencilId;
typedef Hash64 InputAssemblyId;

} // Pipeline
} // Recluse