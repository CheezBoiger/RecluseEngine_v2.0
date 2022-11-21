//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

namespace Recluse {


class GraphicsResource;
class GraphicsResourceView;
class GraphicsSampler;


// DescriptorBind Description, used to describe the resource that will bind to the respective 
// descriptor slot within a given DescriptorSet. The DescriptorSet itself, created with the given layout, is the resource
// that binds to the shaders.
struct DescriptorBindDesc 
{
    DescriptorBindType bindType;        // The Descriptor Bind type.
    ShaderTypeFlags    shaderStages;    // Shader stages accessible to the descriptor set layout.
    U32                binding;         // Binding slot to put our resource.
    U32                numDescriptors;  // Number of descriptors in array.
};

struct DescriptorSetLayoutDesc 
{
    DescriptorBindDesc* pDescriptorBinds;
    U32             numDescriptorBinds;
};

struct DescriptorSetBind 
{
    DescriptorBindType  bindType;
    U32                 binding;
    U32                 descriptorCount;
    union 
    {
        struct 
        {
            GraphicsResource*   buffer;
            U64                 offset;
            U64                 sizeBytes;
        } cb;
        
        struct 
        {
            GraphicsResourceView* pView;
        } srv;
        
        struct 
        { 
            GraphicsSampler* pSampler;
        } sampler;
    };
};

// DescriptorSetLayout is a resource that is used to layout the resources to be bound to the pipeline.
// Essentially it is the root signature that tells the pipeline how to access resources, which resources
// can be accessed by certain shaders, and how to read those bound resources. You will need to create 
// a DescriptorSet, in order to actually bind the resources themselves to the pipeline.
class R_PUBLIC_API DescriptorSetLayout 
{
public:
    DescriptorSetLayout() { }
    virtual ~DescriptorSetLayout() { }
};


// DescriptorSet is a set of resources that are bound, and are used within the pipeline.
// In order to bind our resources, we essentially need to bind them to the DescriptorSet first.
// At the core, DescriptorSets are created via a layout (see DescriptorSetLayout.)
class R_PUBLIC_API DescriptorSet 
{
public:
    // Update the DescriptorSet. This will bind all resources provided, to the DescriptorSet.
    // In order to let shaders and the rendering pipeline see the resources, you must call this update function.
    virtual ErrType update(DescriptorSetBind* pBinds, U32 bindCount) { return RecluseResult_NoImpl; }
};
} // Recluse