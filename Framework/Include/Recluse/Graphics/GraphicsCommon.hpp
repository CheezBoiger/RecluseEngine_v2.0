//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


enum BindType {
    BIND_TYPE_GRAPHICS,
    BIND_TYPE_COMPUTE,
    BIND_TYPE_RAY_TRACE,
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
    RESOURCE_USAGE_DEPTH_STENCIL            = (1 << 9),
    RESOURCE_USAGE_STORAGE_IMAGE            = (1 << 10)
};


typedef U32 ResourceUsageFlags;


enum ResourceViewType {
    RESOURCE_VIEW_TYPE_RENDER_TARGET,
    RESOURCE_VIEW_TYPE_SHADER_RESOURCE,
    RESOURCE_VIEW_TYPE_STORAGE_BUFFER,
    RESOURCE_VIEW_TYPE_STORAGE_IMAGE,
    RESOURCE_VIEW_TYPE_DEPTH_STENCIL
};


enum FrameBuffering {
    FRAME_BUFFERING_SINGLE, // Single immediate run.
    FRAME_BUFFERING_DOUBLE, // VSync
    FRAME_BUFFERING_TRIPLE // Triple buffering.
};


enum R_EXPORT LayerFeatures {
    LAYER_FEATURE_RAY_TRACING_BIT   = (1 << 0),
    LAYER_FEATURE_MESH_SHADING_BIT  = (1 << 1),
    LAYER_FEATURE_DEBUG_VALIDATION_BIT         = (1 << 2),
    LAYER_FEATURE_API_DUMP_BIT      = (1 << 3)
};


typedef U32 EnableLayerFlags;


enum GraphicsAPI {
    GRAPHICS_API_SOFTWARE,
    GRAPHICS_API_VULKAN,
    GRAPHICS_API_OPENGL,
    GRAPHICS_API_D3D11,
    GRAPHICS_API_D3D12
};


enum DescriptorBindType {
    DESCRIPTOR_SHADER_RESOURCE_VIEW,
    DESCRIPTOR_STORAGE_BUFFER,
    DESCRIPTOR_STORAGE_IMAGE,
    DESCRIPTOR_CONSTANT_BUFFER,
    DESCRIPTOR_SAMPLER
};


struct Viewport {
    F32 x;
    F32 y;
    F32 width;
    F32 height;
    F32 minDepth;
    F32 maxDepth;
};


struct Rect {
    F32 x, 
        y, 
        width, 
        height;
};
} // Recluse