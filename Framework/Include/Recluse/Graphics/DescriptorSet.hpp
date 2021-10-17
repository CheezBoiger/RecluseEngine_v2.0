//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

namespace Recluse {


class GraphicsResource;
class GraphicsResourceView;
class GraphicsSampler;


struct DescriptorBindDesc {
    DescriptorBindType bindType;        // The Descriptor Bind type.
    ShaderTypeFlags    shaderStages;    // Shader stages accessible to the descriptor set layout.
    U32                binding;         // Binding slot to put our resource.
    U32                numDescriptors;  // Number of descriptors in array.
};

struct DescriptorSetLayoutDesc {
    DescriptorBindDesc* pDescriptorBinds;
    U32             numDescriptorBinds;
};

struct DescriptorSetBind {
    DescriptorBindType  bindType;
    U32                 binding;
    U32                 descriptorCount;
    union {
        struct {
            GraphicsResource*   buffer;
            U64                 offset;
            U64                 sizeBytes;
        } cb;
        struct {
            GraphicsResourceView* pView;
        } srv;
        struct { 
            GraphicsSampler* pSampler;
        } sampler;
    };
};

class R_PUBLIC_API DescriptorSetLayout {
public:
    DescriptorSetLayout() { }
    virtual ~DescriptorSetLayout() { }
};


class R_PUBLIC_API DescriptorSet {
public:

    virtual ErrType update(DescriptorSetBind* pBinds, U32 bindCount) { return REC_RESULT_NOT_IMPLEMENTED; }
};
} // Recluse