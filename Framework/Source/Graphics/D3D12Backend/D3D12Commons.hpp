//
#pragma once

#include "Win32/Win32Common.hpp"

#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/Shader.hpp"

// D3D12 headers.
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>

#define R_CHANNEL_D3D12 "D3D12"

// Useful information about differences between D3D12 and Vulkan:
// https://asawicki.info/articles/memory_management_vulkan_direct3d_12.php5
// Proc access to these functions when we query for instance.


namespace Recluse {


class D3D12GraphicsObject : public IGraphicsObject
{
public:
	virtual ~D3D12GraphicsObject() { }

	GraphicsAPI getApi() const override { return GraphicsApi_Direct3D12; }
};


struct D3D12MemoryPool 
{
    ID3D12Heap* pHeap;
    U64             sizeInBytes;
};


struct D3D12MemoryObject 
{
    ID3D12Resource*         pResource;
    U64                     sizeInBytes;
    UPtr                    basePtr;
    U32                     allocatorIndex;
    ResourceMemoryUsage     usage;
};


// Get the native resource state.
extern D3D12_RESOURCE_STATES getNativeResourceState(Recluse::ResourceState state);
extern D3D12_RTV_DIMENSION getRtvDimension(Recluse::ResourceViewDimension dimension);
extern D3D12_DSV_DIMENSION getDsvDimension(Recluse::ResourceViewDimension dimension);
extern D3D12_UAV_DIMENSION getUavDimension(Recluse::ResourceViewDimension dimension);
extern D3D12_SRV_DIMENSION getSrvDimension(Recluse::ResourceViewDimension dimension);
extern D3D12_COMPARISON_FUNC getNativeComparisonFunction(Recluse::CompareOp compareOp);
extern D3D12_SHADER_VISIBILITY getShaderVisibilityFlags(Recluse::ShaderStageFlags shaderStageFlags);

} // Recluse


namespace Dxgi {

// Check format size, this uses DXGI format.
extern SIZE_T getNativeFormatSize(DXGI_FORMAT format);

// Get the native format.
extern DXGI_FORMAT getNativeFormat(Recluse::ResourceFormat format);
} // Dxgi