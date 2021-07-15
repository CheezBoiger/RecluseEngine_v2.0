//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"

namespace Recluse {


class RenderPass;

struct PipelineStateDesc {
    RenderPass*     pRenderPass;
    DescriptorSetLayout** ppDescriptorLayouts;
    U32                   numDescriptorSetLayouts;
};


struct GraphicsPipelineStateDesc : public PipelineStateDesc {

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