//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"

namespace Recluse {


class RenderPass;
class Shader;

struct PipelineStateDesc {
    RenderPass*     pRenderPass;
    DescriptorSetLayout** ppDescriptorLayouts;
    U32                   numDescriptorSetLayouts;
};


struct ShaderModule {
    void* byteCode;
    U32 szBytes;
};


enum InputRate {
    INPUT_RATE_PER_VERTEX,
    INPUT_RATE_PER_INSTANCE
};


enum PrimitiveTopology {
    PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    PRIMITIVE_TOPOLOGY_POINT_LIST,
    PRIMITIVE_TOPOLOGY_LINE_LIST,
    PRIMITIVE_TOPOLOGY_LINE_STRIP
};


enum FrontFace {
    FRONT_FACE_COUNTER_CLOCKWISE,
    FRONT_FACE_CLOCKWISE
};


enum StencilOp {
    STENCIL_OP_KEEP,
    STENCIL_OP_ZERO,
    STENCIL_OP_REPLACE,
    STENCIL_OP_INCREMENT_AND_CLAMP,
    STENCIL_OP_DECREMENT_AND_CLAMP,
    STENCIL_OP_INVERT,
    STENCIL_OP_INCREMENT_AND_WRAP,
    STENCIL_OP_DECREMENT_AND_WRAP
};


enum PolygonMode {
    POLYGON_MODE_FILL,
    POLYGON_MODE_LINE,
    POLYGON_MODE_POINT
};


enum CullMode {
    CULL_MODE_FRONT,
    CULL_MODE_BACK,
    CULL_MODE_FRONT_AND_BACK
};


enum CompareOp {
    COMPARE_OP_NEVER,
    COMPARE_OP_LESS,
    COMPARE_OP_EQUAL,
    COMPARE_OP_LESS_OR_EQUAL,
    COMPARE_OP_GREATER,
    COMPARE_OP_NOT_EQUAL,
    COMPARE_OP_GREATER_OR_EQUAL,
    COMPARE_OP_ALWAYS
};


struct VertexAttribute {
    U32 loc;
    U32 offset;             // offset within the vertex attribute.
    ResourceFormat format;
    char* semantic;
    
};


struct VertexBinding {
    U32 binding;
    U32 stride;                         // Data Step rate between consecutive elements.
    InputRate inputRate;
    VertexAttribute* pVertexAttributes;
    U32 numVertexAttributes;
};


struct GraphicsPipelineStateDesc : public PipelineStateDesc {
    Shader* pVS;
    Shader* pHS;
    Shader* pDS;
    Shader* pGS;
    Shader* pPS;
    
    struct VertexInput {
        VertexBinding* pVertexBindings;
        U32             numVertexBindings;
    } vi;

    struct DepthStencil {
        B8  depthBoundsTestEnable;
        B8  depthTestEnable;
        B8  stencilTestEnable;
        B8  depthWriteEnable;
        F32 minDepthBounds;
        F32 maxDepthBounds;
    } ds;

    struct RasterState {
        CullMode cullMode;
        FrontFace frontFace;
        PolygonMode polygonMode;
        F32 lineWidth;
        F32 depthBiasClamp;
        F32 depthBiasConstantFactor;
        F32 depthBiasSlopFactor;
        B32 depthClampEnable : 1, 
            depthBiasEnable : 1;
    } raster;

    struct BlendState {
        
    } blend;

    PrimitiveTopology primitiveTopology;

    
};


struct ComputePipelineStateDesc : public PipelineStateDesc {

};


struct RayTracingPipelineStateDesc : public PipelineStateDesc {

};


class PipelineState {
public:
    virtual ~PipelineState() { }
};
} // Recluse