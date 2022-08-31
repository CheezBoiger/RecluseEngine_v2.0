//
#pragma once

#include "Win32/Win32Common.hpp"

#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

// D3D12 headers.
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>

#define R_CHANNEL_D3D12 "D3D12"


namespace Recluse {


class D3D12GraphicsObject : public IGraphicsObject
{
public:
	virtual ~D3D12GraphicsObject() { }

	GraphicsAPI getApi() const override { return GRAPHICS_API_D3D12; }
};


struct D3D12MemoryPool 
{
    ID3D12Heap* pHeap;
    U64             sizeInBytes;
};


struct D3D12MemoryObject 
{
    ID3D12Resource* pResource;
    U64                     sizeInBytes;
    PtrType                 basePtr;
};
} // Recluse


namespace Dxgi {

// Check format size, this uses DXGI format.
extern SIZE_T getNativeFormatSize(DXGI_FORMAT format);

// Get the native format.
extern DXGI_FORMAT getNativeFormat(Recluse::ResourceFormat format);

// Get the native resource state.
extern D3D12_RESOURCE_STATES getNativeResourceState(Recluse::ResourceState state, bool isPixelShader);
} // Dxgi