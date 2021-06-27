//
#pragma once

#include "Core/Types.hpp"
#include "CommandQueue.hpp"

namespace Recluse {


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
    RESOURCE_DIMENSION_3D,
    RESOURCE_DIMENSION_1D_ARRAY,
    RESOURCE_DIMENSION_2D_ARRAY
};


enum ResourceMemoryUsage {
  RESOURCE_MEMORY_USAGE_CPU_ONLY,
  RESOURCE_MEMORY_USAGE_GPU_ONLY,
  RESOURCE_MEMORY_USAGE_CPU_TO_GPU,
  RESOURCE_MEMORY_USAGE_GPU_TO_CPU
};


struct GraphicsResourceDescription {
    U32                 width;
    U32                 height;
    U32                 arrayLevels;
    U32                 mipLevels;
    ResourceDimension   dimension;
    U32                 format;
    U32                 layout;
    ResourceMemoryUsage memoryUsage;
};


struct MemoryReserveDesc {
    U64 hostBufferMemoryBytes;      // Host memory for buffers.
    U64 hostTextureMemoryBytes;     // Host memory for textures.
    U64 deviceBufferMemoryBytes;    // Device memory for buffers.
    U64 deviceTextureMemoryBytes;   // Device memory for textures.
    U64 uploadMemoryBytes;          // Memory used as upload to device memory.
    U64 readbackMemoryBytes;        // Memory used as reading back from device memory.
};


class R_EXPORT GraphicsDevice {
public:
    // Reserve memory to be used for graphics resources.
    virtual ErrType reserveMemory(const MemoryReserveDesc& desc) { return 0; }

    virtual ErrType createResource(GraphicsResource** ppResource, GraphicsResourceDescription* pDesc) 
        { return 0; }
    
    virtual ErrType createResourceView() { return 0; }

    virtual ErrType createCommandList() { return 0; }
    virtual ErrType createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type) { return 0; }
    virtual ErrType createPipeline() { return 0; }

    virtual ErrType createSwapchain(GraphicsSwapchain** swapchain,
        const SwapchainCreateDescription* pDescription) { return 0; }

    virtual ErrType destroySwapchain(GraphicsSwapchain* pSwapchain) { return 0; }
    virtual ErrType destroyCommandQueue(GraphicsQueue* pQueue) { return 0; }
private:
};


class GraphicsSwapchain {
public:
    R_EXPORT virtual ~GraphicsSwapchain() { }

    // Rebuild the swapchain if need be. Pass in NULL to rebuild the swapchain as is.
    // Be sure to update any new frame info and handles that are managed by the front engine!
    R_EXPORT virtual ErrType rebuild(const SwapchainCreateDescription* pDesc) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    // Present the current image.
    R_EXPORT virtual ErrType present() { return REC_RESULT_NOT_IMPLEMENTED; }

    // Get the current frame index, updates after every present call.
    R_EXPORT virtual U32 getCurrentFrameIndex() { return 0; }

    virtual GraphicsResource* getFrame(U32 idx) = 0;
    virtual GraphicsResourceView* getFrameView(U32 idx) = 0;
};
} // Recluse