//
#pragma once

#include "Win32/Win32Common.hpp"

#include "Recluse/Graphics/Format.hpp"

// D3D12 headers.
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>

#define R_CHANNEL_D3D12 "D3D12"


namespace Dxgi {


extern SIZE_T getNativeFormatSize(Recluse::ResourceFormat format);
extern DXGI_FORMAT getNativeFormat(Recluse::ResourceFormat format);
} // Dxgi