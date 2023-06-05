//
#include "D3D12Commons.hpp"
#include "D3D12ResourceView.hpp"
#include "D3D12Device.hpp"
#include "D3D12DescriptorTableManager.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


U64 D3D12GraphicsResourceView::kViewCreationCounter = 0;
MutexGuard D3D12GraphicsResourceView::kViewMutex = MutexGuard("D3D12GraphicsResourceViewMutex");


ResultCode D3D12GraphicsResourceView::initialize(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);
    ID3D12Device* pNative = pDevice->get();
    R_ASSERT(pNative != NULL);
    ResourceViewDescription resourceViewDescription = getDesc();

    switch (getDesc().type) 
    {   
        case ResourceViewType_RenderTarget:
        {
            rtvDescription.ViewDimension = getRtvDimension(resourceViewDescription.dimension);
            rtvDescription.Format = Dxgi::getNativeFormat(resourceViewDescription.format);
            break;
        }

        case ResourceViewType_DepthStencil:
        {
            dsvDescription.ViewDimension = getDsvDimension(resourceViewDescription.dimension);
            dsvDescription.Format = Dxgi::getNativeFormat(resourceViewDescription.format);
            break;
        }

        case ResourceViewType_ShaderResource:
        {
            srvDescription.ViewDimension = getSrvDimension(resourceViewDescription.dimension);
            srvDescription.Format = Dxgi::getNativeFormat(resourceViewDescription.format);
            break;
        }

        case ResourceViewType_UnorderedAccess:
        {
            uavDescription.ViewDimension = getUavDimension(resourceViewDescription.dimension);
            uavDescription.Format = Dxgi::getNativeFormat(resourceViewDescription.format);
            break;
        }

        default:
            break;
    }

    return RecluseResult_Ok;
}


R_INTERNAL D3D12_TEXTURE_ADDRESS_MODE getAddressMode(SamplerAddressMode addressMode)
{
    switch (addressMode)
    {
        case SamplerAddressMode_ClampToBorder:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case SamplerAddressMode_ClampToEdge:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case SamplerAddressMode_MirrorClampToEdge:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        case SamplerAddressMode_MirroredRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case SamplerAddressMode_Repeat:
        default:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}


typedef Filter MinFilter;
typedef Filter MagFilter;
typedef SamplerMipMapMode MipFilter;

typedef U32 MinMagMipFilterType;

std::unordered_map<MinMagMipFilterType, D3D12_FILTER> g_nativeFilterMap = {
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Linear)),    D3D12_FILTER_MIN_MAG_MIP_LINEAR },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Nearest)),   D3D12_FILTER_MIN_MAG_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Nearest)),   D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Linear)),    D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Nearest)),   D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Linear)),    D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR }
};

R_INTERNAL D3D12_FILTER getFilterMode(Filter minFilter, Filter magFilter, SamplerMipMapMode mipMode)
{
    MinMagMipFilterType filterType = makeBitset32(0, 3, minFilter) 
                                        | makeBitset32(3, 3, magFilter)
                                        | makeBitset32(3, 6, mipMode);
    auto it = g_nativeFilterMap.find(filterType);
    if (it != g_nativeFilterMap.end())
        return it->second;
    return D3D12_FILTER_ANISOTROPIC;
}


ResultCode D3D12Sampler::initialize(D3D12Device* pDevice, const SamplerDescription& desc)
{
    R_ASSERT(pDevice != NULL);

    D3D12_SAMPLER_DESC samplerDesc = { };
    samplerDesc.AddressU                = getAddressMode(desc.addressModeU);
    samplerDesc.AddressV                = getAddressMode(desc.addressModeV);
    samplerDesc.AddressW                = getAddressMode(desc.addressModeW);
    samplerDesc.BorderColor;
    samplerDesc.ComparisonFunc          = getNativeComparisonFunction(desc.compareOp);
    samplerDesc.Filter                  = getFilterMode(desc.minFilter, desc.magFilter, desc.mipMapMode);
    samplerDesc.MaxAnisotropy           = desc.maxAnisotropy;
    samplerDesc.MinLOD                  = desc.minLod;
    samplerDesc.MaxLOD                  = desc.maxLod;
    samplerDesc.MipLODBias              = desc.mipLodBias;

    m_samplerDesc = samplerDesc;

    return RecluseResult_NoImpl;
}


ResultCode D3D12Sampler::destroy(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);
    ID3D12Device* device = pDevice->get();
    return RecluseResult_NoImpl;
}
} // Recluse