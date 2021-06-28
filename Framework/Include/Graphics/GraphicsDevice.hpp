//
#pragma once

#include "Core/Types.hpp"
#include "CommandQueue.hpp"
#include "Format.hpp"

namespace Recluse {

class GraphicsCommandList;
class GraphicsAdapter;
class GraphicsResource;
class GraphicsResourceView;
class GraphicsPipeline;
class GraphicsSwapchain;
class GraphicsContext;
class GraphicsQueue;

struct SwapchainCreateDescription {
    U32 desiredFrames;
    U32 renderWidth;
    U32 renderHeight;
    GraphicsQueue* pBackbufferQueue;
};


enum ResourceDimension {
    RESOURCE_DIMENSION_BUFFER,
    RESOURCE_DIMENSION_1D,
    RESOURCE_DIMENSION_2D,
    RESOURCE_DIMENSION_3D
};

enum ResourceViewDimension {
    RESOURCE_VIEW_DIMENSION_BUFFER,
    RESOURCE_VIEW_DIMENSION_1D,
    RESOURCE_VIEW_DIMENSION_1D_ARRAY,
    RESOURCE_VIEW_DIMENSION_2D,
    RESOURCE_VIEW_DIMENSION_2D_MS,
    RESOURCE_VIEW_DIMENSION_2D_ARRAY,
    RESOURCE_VIEW_DIMENSION_3D,
    RESOURCE_VIEW_DIMENSION_CUBE,
    RESOURCE_VIEW_DIMENSION_CUBE_ARRAY
};


enum ResourceMemoryUsage {
    RESOURCE_MEMORY_USAGE_CPU_ONLY,
    RESOURCE_MEMORY_USAGE_GPU_ONLY,
    RESOURCE_MEMORY_USAGE_CPU_TO_GPU,
    RESOURCE_MEMORY_USAGE_GPU_TO_CPU,
    RESOURCE_MEMORY_USAGE_COUNT = (RESOURCE_MEMORY_USAGE_GPU_TO_CPU + 1)
};


enum ResourceUsage {
    RESOURCE_USAGE_VERTEX_BUFFER            = (1 << 0),
    RESOURCE_USAGE_INDEX_BUFFER             = (1 << 1),
    RESOURCE_USAGE_STORAGE_BUFFER           = (1 << 2),
    RESOURCE_USAGE_RENDER_TARGET            = (1 << 3),
    RESOURCE_USAGE_SHADER_RESOURCE          = (1 << 4),
    RESOURCE_USAGE_CONSTANT_BUFFER          = (1 << 5),
    RESOURCE_USAGE_TRANSFER_DESTINATION     = (1 << 6),
    RESOURCE_USAGE_TRANSFER_SOURCE          = (1 << 7),
    RESOURCE_USAGE_INDIRECT                 = (1 << 8),
    RESOURCE_USAGE_DEPTH_STENCIL            = (1 << 9)
};

typedef U32 ResourceUsageFlags;

struct GraphicsResourceDescription {
    U32                 width;
    U32                 height;
    U32                 depth;
    U32                 arrayLevels;
    U32                 mipLevels;
    ResourceDimension   dimension;
    ResourceFormat      format;
    U32                 samples;
    ResourceMemoryUsage memoryUsage;
    ResourceUsageFlags  usage;
};


struct MemoryReserveDesc {
    U64 bufferPools[RESOURCE_MEMORY_USAGE_COUNT];
    U64 texturePools[RESOURCE_MEMORY_USAGE_COUNT];
};


class R_EXPORT GraphicsDevice {
public:
    // Reserve memory to be used for graphics resources.
    virtual ErrType reserveMemory(const MemoryReserveDesc& desc) { return 0; }

    virtual ErrType createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc) 
        { return 0; }
    
    virtual ErrType createResourceView() { return 0; }

    virtual ErrType createCommandList(GraphicsCommandList** pList) { return 0; }
    virtual ErrType createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type) { return 0; }
    virtual ErrType createPipeline() { return 0; }

    virtual ErrType createSwapchain(GraphicsSwapchain** swapchain,
        const SwapchainCreateDescription& pDescription) { return 0; }

    virtual ErrType destroySwapchain(GraphicsSwapchain* pSwapchain) { return 0; }
    virtual ErrType destroyCommandQueue(GraphicsQueue* pQueue) { return 0; }
    virtual ErrType destroyResource(GraphicsResource* pResource) { return 0; }
    virtual ErrType destroyCommandList(GraphicsCommandList* pList) { return 0; }
private:
};


class GraphicsSwapchain {
public:
    R_EXPORT virtual ~GraphicsSwapchain() { }

    // Rebuild the swapchain if need be. Pass in NULL to rebuild the swapchain as is.
    // Be sure to update any new frame info and handles that are managed by the front engine!
    R_EXPORT virtual ErrType rebuild(const SwapchainCreateDescription& pDesc) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    // Present the current image.
    R_EXPORT virtual ErrType present() { return REC_RESULT_NOT_IMPLEMENTED; }

    // Get the current frame index, updates after every present call.
    R_EXPORT virtual U32 getCurrentFrameIndex() { return 0; }

    virtual GraphicsResource* getFrame(U32 idx) = 0;
    virtual GraphicsResourceView* getFrameView(U32 idx) = 0;
};
} // Recluse