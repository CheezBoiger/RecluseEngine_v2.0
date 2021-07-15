//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"

namespace Recluse {


enum DescriptorBindType {
    DESCRIPTOR_SHADER_RESOURCE_VIEW,
    DESCRIPTOR_STORAGE_BUFFER,
    DESCRIPTOR_STORAGE_IMAGE,
    DESCRIPTOR_CONSTANT_BUFFER,
    DESCRIPTOR_SAMPLER
};


struct DescriptorBind {
    DescriptorBindType bindType;        // The Descriptor Bind type.
    ShaderTypeFlags    shaderStages;    // Shader stages accessible to the descriptor set layout.
    U32                binding;         // Binding slot to put our resource.
    U32                numDescriptors;  // Number of descriptors in array.
};

struct DescriptorSetLayoutDesc {
    DescriptorBind* pDescriptorBinds;
    U32             numDescriptorBinds;
};

class DescriptorSetLayout {
public:
    DescriptorSetLayout();
    virtual ~DescriptorSetLayout();
};


class DescriptorSet {

};
} // Recluse