//
#include "D3D12Commons.hpp"
#include "D3D12ResourceView.hpp"
#include "D3D12Device.hpp"
#include "D3D12DescriptorTableManager.hpp"

#include "Recluse/Messaging.hpp"
#include <unordered_map>

namespace Recluse {
namespace D3D12 {

U64 D3D12GraphicsResourceView::kViewCreationCounter = 0;
U64 D3D12Sampler::kSamplerCreationCounter = 0;
MutexGuard D3D12Sampler::kSamplerMutex = MutexGuard("D3D12SamplerMutex");
MutexGuard D3D12GraphicsResourceView::kViewMutex = MutexGuard("D3D12GraphicsResourceViewMutex");
std::unordered_map<ResourceViewId, D3D12GraphicsResourceView*> g_resourceViewMap;
std::unordered_map<Hash64, D3D12Sampler> g_samplerMap;


namespace DescriptorViews {

ResourceViewId makeResourceView(D3D12Device* pDevice, ID3D12Resource* pResource, const ResourceViewDescription& description)
{
    ResourceViewId resourceViewId = 0;
    D3D12GraphicsResourceView* pView = new D3D12GraphicsResourceView(description);
    pView->initialize(pDevice, pResource);
    g_resourceViewMap.insert(std::make_pair(pView->getId(), pView));
    return pView->getId();
}


ResultCode destroyResourceView(D3D12Device* pDevice, ResourceViewId resourceId)
{
    auto iter = g_resourceViewMap.find(resourceId);
    if (iter != g_resourceViewMap.end())
    {
        iter->second->release(pDevice);
        delete iter->second;
        g_resourceViewMap.erase(iter);
        return RecluseResult_Ok;
    }

    return RecluseResult_NotFound;
}


D3D12GraphicsResourceView* findResourceView(ResourceViewId id)
{
    auto iter = g_resourceViewMap.find(id);
    if (iter == g_resourceViewMap.end())
    {
        return nullptr;
    }
    return iter->second;
}


void clearAll(D3D12Device* pDevice)
{
    R_DEBUG(R_CHANNEL_D3D12, "Clearing all resource views...");
    for (auto iter : g_resourceViewMap)
    {
        iter.second->release(pDevice);
        delete iter.second;
    }
    g_resourceViewMap.clear();
    for (auto iter : g_samplerMap)
    {
        iter.second.destroy(pDevice);
    }
    g_samplerMap.clear();
}


D3D12Sampler* makeSampler(D3D12Device* pDevice, const SamplerDescription& description)
{
    Hash64 hash = recluseHashFast(&description, sizeof(SamplerDescription));
    auto iter = g_samplerMap.find(hash);
    if (iter == g_samplerMap.end())
    {
        D3D12Sampler sampler = D3D12Sampler();
        sampler.initialize(pDevice, description);
        sampler.generateId();
        g_samplerMap.insert(std::make_pair(hash, sampler));
        return &g_samplerMap[hash];
    }
    else
    {
        return &iter->second;
    }
}

ResultCode destroySampler(D3D12Device* pDevice, D3D12Sampler* sampler)
{
    Hash64 hash = sampler->getHash();
    auto iter = g_samplerMap.find(hash);
    if (iter != g_samplerMap.end())
    {
        iter->second.destroy(pDevice);
        g_samplerMap.erase(iter);
    }
    return RecluseResult_Ok;
}


D3D12_CPU_DESCRIPTOR_HANDLE makeCbv(D3D12Device* pDevice, D3D12_GPU_VIRTUAL_ADDRESS address, U32 sizeBytes)
{
    DescriptorHeapAllocationManager* manager = pDevice->getDescriptorHeapManager();
    R_ASSERT(manager);
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = { };
    desc.BufferLocation = address;
    desc.SizeInBytes = sizeBytes;
    D3D12_CPU_DESCRIPTOR_HANDLE handle = manager->allocateConstantBufferView(desc);
    return handle;
}


ResultCode destroyCbv(D3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    DescriptorHeapAllocationManager* manager = pDevice->getDescriptorHeapManager();
    R_ASSERT(manager);
    return manager->freeConstantBufferView(handle);
}
} // DescriptorViews


R_INTERNAL
void fillRenderTargetViewDescription(D3D12_RENDER_TARGET_VIEW_DESC& nativeDesc, const ResourceViewDescription& description)
{
    switch (nativeDesc.ViewDimension)
    {
        case D3D12_RTV_DIMENSION_BUFFER:
        {
            nativeDesc.Buffer.FirstElement = description.firstElement;
            nativeDesc.Buffer.NumElements = description.numElements;
            break;
        }
        case D3D12_RTV_DIMENSION_TEXTURE1D:
        {
            break;
        }
        case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
        {
            nativeDesc.Texture1DArray.FirstArraySlice = description.baseArrayLayer;
            nativeDesc.Texture1DArray.ArraySize = description.layerCount;
            nativeDesc.Texture1DArray.MipSlice = description.baseMipLevel;
            break;
        }
        default:
        case D3D12_RTV_DIMENSION_TEXTURE2D:
        {
            nativeDesc.Texture2D.MipSlice = description.baseMipLevel;
            nativeDesc.Texture2D.PlaneSlice = 0;
            break;
        }
        case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
        {
            nativeDesc.Texture2DArray.ArraySize = description.layerCount;
            nativeDesc.Texture2DArray.FirstArraySlice = description.baseArrayLayer;
            nativeDesc.Texture2DArray.MipSlice = description.baseMipLevel;
            nativeDesc.Texture2DArray.PlaneSlice = 0;
            break;
        }
        case D3D12_RTV_DIMENSION_TEXTURE3D:
        {
            break;
        }
    }
}


R_INTERNAL
void fillDepthStencilViewDescription(D3D12_DEPTH_STENCIL_VIEW_DESC& nativeDesc, const ResourceViewDescription& description)
{
    nativeDesc.Flags = D3D12_DSV_FLAG_NONE;
    switch (nativeDesc.ViewDimension)
    {
        case D3D12_DSV_DIMENSION_TEXTURE1D:
        {
            nativeDesc.Texture1D.MipSlice = description.baseMipLevel;
            break;
        }
        case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
        {
            nativeDesc.Texture1DArray.ArraySize = description.layerCount;
            nativeDesc.Texture1DArray.FirstArraySlice = description.baseArrayLayer;
            nativeDesc.Texture1DArray.MipSlice = description.baseMipLevel;
            break;
        }
        default:
        case D3D12_DSV_DIMENSION_TEXTURE2D:
        {
            nativeDesc.Texture2D.MipSlice = description.baseMipLevel;
            break;
        }
        case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
        {
            nativeDesc.Texture2DArray.ArraySize = description.layerCount;
            nativeDesc.Texture2DArray.FirstArraySlice = description.baseArrayLayer;
            nativeDesc.Texture2DArray.MipSlice = description.baseMipLevel;
            break;
        }
    }
}


R_INTERNAL
void fillShaderResourceViewDescription(D3D12_SHADER_RESOURCE_VIEW_DESC& nativeDesc, const ResourceViewDescription& description)
{
     nativeDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    switch (nativeDesc.ViewDimension)
    {
        default:
        case D3D12_SRV_DIMENSION_BUFFER:
        {
            nativeDesc.Buffer.FirstElement = description.firstElement;
            nativeDesc.Buffer.NumElements = description.numElements;
            nativeDesc.Buffer.StructureByteStride = description.byteStride;
            nativeDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            break;
        }
        case D3D12_SRV_DIMENSION_TEXTURE1D:
        {
            nativeDesc.Texture1D.MipLevels = description.mipLevelCount;
            nativeDesc.Texture1D.MostDetailedMip = description.baseMipLevel;
            nativeDesc.Texture1D.ResourceMinLODClamp = 0.f;
            break;
        }
        case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
        {
            nativeDesc.Texture1DArray.ArraySize = description.layerCount;
            nativeDesc.Texture1DArray.FirstArraySlice = description.baseArrayLayer;
            nativeDesc.Texture1DArray.MipLevels = description.mipLevelCount;
            nativeDesc.Texture1DArray.MostDetailedMip = description.baseMipLevel;
            nativeDesc.Texture1DArray.ResourceMinLODClamp = 0.f;
            break;
        }
        case D3D12_SRV_DIMENSION_TEXTURE2D:
        {
            nativeDesc.Texture2D.MipLevels = description.mipLevelCount;
            nativeDesc.Texture2D.MostDetailedMip = description.baseMipLevel;
            nativeDesc.Texture2D.PlaneSlice = 0;
            nativeDesc.Texture2D.ResourceMinLODClamp = 0.f;
            break;
        }
        case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
        {
            nativeDesc.Texture2DArray.ArraySize = description.layerCount;
            nativeDesc.Texture2DArray.FirstArraySlice = description.baseArrayLayer;
            nativeDesc.Texture2DArray.MipLevels = description.mipLevelCount;
            nativeDesc.Texture2DArray.MostDetailedMip = description.baseMipLevel;
            nativeDesc.Texture2DArray.PlaneSlice = 0;
            nativeDesc.Texture2DArray.ResourceMinLODClamp = 0.f;
            break;
        }
        case D3D12_SRV_DIMENSION_TEXTURE3D:
        {
            nativeDesc.Texture3D.MipLevels = description.mipLevelCount;
            nativeDesc.Texture3D.MostDetailedMip = description.baseMipLevel;
            nativeDesc.Texture3D.ResourceMinLODClamp = 0.f;
            break;
        }
        case D3D12_SRV_DIMENSION_TEXTURECUBE:
        {
            nativeDesc.TextureCube.MipLevels = description.mipLevelCount;
            nativeDesc.TextureCube.MostDetailedMip = description.baseMipLevel;
            nativeDesc.TextureCube.ResourceMinLODClamp = 0.f;
            break;
        }   
        case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
        {
            nativeDesc.TextureCubeArray.MipLevels = description.mipLevelCount;
            nativeDesc.TextureCubeArray.MostDetailedMip = description.baseMipLevel;
            nativeDesc.TextureCubeArray.First2DArrayFace = description.baseArrayLayer / 6;
            nativeDesc.TextureCubeArray.NumCubes = description.layerCount / 6;
            nativeDesc.TextureCubeArray.ResourceMinLODClamp = 0.f;
            break;
        }
    }
}


R_INTERNAL
void fillUnorderedAccessViewDescription(D3D12_UNORDERED_ACCESS_VIEW_DESC& nativeDesc, const ResourceViewDescription& description)
{
    switch (nativeDesc.ViewDimension)
    {
        default:
        case D3D12_UAV_DIMENSION_BUFFER:
        {
            nativeDesc.Buffer.FirstElement = description.firstElement;
            nativeDesc.Buffer.NumElements = description.numElements;
            nativeDesc.Buffer.StructureByteStride = description.byteStride;
            nativeDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            break;
        }
        case D3D12_UAV_DIMENSION_TEXTURE1D:
        {
            nativeDesc.Texture1D.MipSlice = description.baseMipLevel;
            break;
        }
        case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
        {
            nativeDesc.Texture1DArray.ArraySize = description.layerCount;
            nativeDesc.Texture1DArray.FirstArraySlice = description.baseArrayLayer;
            nativeDesc.Texture1DArray.MipSlice = description.baseMipLevel;
            break;
        }
        case D3D12_UAV_DIMENSION_TEXTURE2D:
        {
            nativeDesc.Texture2D.MipSlice = description.baseMipLevel;
            nativeDesc.Texture2D.PlaneSlice = 0;
            break;
        }
        case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
        {
            nativeDesc.Texture2DArray.ArraySize = description.layerCount;
            nativeDesc.Texture2DArray.FirstArraySlice = description.baseArrayLayer;
            nativeDesc.Texture2DArray.MipSlice = description.baseMipLevel;
            nativeDesc.Texture2DArray.PlaneSlice = 0;
            break;
        }
        case D3D12_UAV_DIMENSION_TEXTURE3D:
        {
            nativeDesc.Texture3D.FirstWSlice = 0;
            nativeDesc.Texture3D.MipSlice = description.baseMipLevel;
            nativeDesc.Texture3D.WSize = 0;
            break;
        }
    }
}


ResultCode D3D12GraphicsResourceView::initialize(D3D12Device* pDevice, ID3D12Resource* pResource)
{
    R_ASSERT(pDevice != NULL);
    ResourceViewDescription resourceViewDescription = getDesc();
    DescriptorHeapAllocationManager* heapManager = pDevice->getDescriptorHeapManager();

    if (resourceViewDescription.dimension != ResourceViewDimension_Buffer)
    {
        R_ASSERT(resourceViewDescription.format != ResourceFormat_Unknown, "Resource format must not be UNKNOWN prior to creation!!");
    }

    switch (getDesc().type) 
    {   
        case ResourceViewType_RenderTarget:
        {
            D3D12_RENDER_TARGET_VIEW_DESC rtvDescription;
            rtvDescription.ViewDimension = getRtvDimension(resourceViewDescription.dimension);
            rtvDescription.Format = Dxgi::getNativeFormat(resourceViewDescription.format);
            fillRenderTargetViewDescription(rtvDescription, resourceViewDescription);
            m_handle = heapManager->allocateRenderTargetView(pResource, rtvDescription);
            break;
        }

        case ResourceViewType_DepthStencil:
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescription;
            dsvDescription.ViewDimension = getDsvDimension(resourceViewDescription.dimension);
            dsvDescription.Format = Dxgi::getNativeFormat(resourceViewDescription.format);
            fillDepthStencilViewDescription(dsvDescription, resourceViewDescription);
            m_handle = heapManager->allocateDepthStencilView(pResource, dsvDescription);
            break;
        }

        case ResourceViewType_ShaderResource:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription;
            srvDescription.ViewDimension = getSrvDimension(resourceViewDescription.dimension);
            srvDescription.Format = Dxgi::getNativeFormat(resourceViewDescription.format);
            fillShaderResourceViewDescription(srvDescription, resourceViewDescription);
            m_handle = heapManager->allocateShaderResourceView(pResource, srvDescription);
            break;
        }

        case ResourceViewType_UnorderedAccess:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescription;
            uavDescription.ViewDimension = getUavDimension(resourceViewDescription.dimension);
            uavDescription.Format = Dxgi::getNativeFormat(resourceViewDescription.format);
            fillUnorderedAccessViewDescription(uavDescription, resourceViewDescription);
            m_handle = heapManager->allocateUnorderedAccessView(pResource, uavDescription);
            break;
        }

        default:
            break;
    }

    generateId();

    return RecluseResult_Ok;
}


ResultCode D3D12GraphicsResourceView::release(D3D12Device* pDevice)
{
    DescriptorHeapAllocationManager* heapManager    = pDevice->getDescriptorHeapManager();
    const ResourceViewDescription& description      = getDesc();
    ResultCode result                               = RecluseResult_Ok;
    switch (description.type)
    {
        case ResourceViewType_RenderTarget:
        {
            result = heapManager->freeRenderTargetView(m_handle);
            break;
        }
        case ResourceViewType_DepthStencil:
        {
            result = heapManager->freeDepthStencilView(m_handle);
            break;
        }
        case ResourceViewType_ShaderResource:
        {
            result = heapManager->freeShaderResourceView(m_handle);
            break;
        }
        case ResourceViewType_UnorderedAccess:
        {
            result = heapManager->freeUnorderedAccessView(m_handle);
            break;
        }
        default:
            R_NO_IMPL();
            break;
    }
    m_handle = DescriptorTable::invalidCpuAddress;
    return result;
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
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Linear)  | makeBitset32(9, 1, 0)),   D3D12_FILTER_MIN_MAG_MIP_LINEAR },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Nearest) | makeBitset32(9, 1, 0)),   D3D12_FILTER_MIN_MAG_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Nearest) | makeBitset32(9, 1, 0)),   D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Linear)  | makeBitset32(9, 1, 0)),   D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Nearest) | makeBitset32(9, 1, 0)),   D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Linear)  | makeBitset32(9, 1, 0)),   D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Nearest) | makeBitset32(9, 1, 0)),   D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Linear)  | makeBitset32(9, 1, 0)),   D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR },

    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Linear)  | makeBitset32(9, 1, 1)),   D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Nearest) | makeBitset32(9, 1, 1)),   D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Nearest) | makeBitset32(9, 1, 1)),   D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Linear)  | makeBitset32(9, 1, 1)),   D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
    { (makeBitset32(0, 3, Filter_Linear)    | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Nearest) | makeBitset32(9, 1, 1)),   D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Nearest)    | makeBitset32(6, 3, SamplerMipMapMode_Linear)  | makeBitset32(9, 1, 1)),   D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Nearest) | makeBitset32(9, 1, 1)),   D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT },
    { (makeBitset32(0, 3, Filter_Nearest)   | makeBitset32(3, 3, Filter_Linear)     | makeBitset32(6, 3, SamplerMipMapMode_Linear)  | makeBitset32(9, 1, 1)),   D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR }
};

R_INTERNAL D3D12_FILTER getFilterMode(Filter minFilter, Filter magFilter, SamplerMipMapMode mipMode, Bool isCompare)
{
    MinMagMipFilterType filterType = makeBitset32(0, 3, minFilter) 
                                        | makeBitset32(3, 3, magFilter)
                                        | makeBitset32(6, 3, mipMode)
                                        | makeBitset32(9, 1, isCompare);
    auto it = g_nativeFilterMap.find(filterType);
    if (it != g_nativeFilterMap.end())
        return it->second;
    return D3D12_FILTER_ANISOTROPIC;
}


ResultCode D3D12Sampler::initialize(D3D12Device* pDevice, const SamplerDescription& desc)
{
    R_ASSERT(pDevice != NULL);
    const Bool isCompare                = (desc.compareOp != CompareOp_Never);
    D3D12_SAMPLER_DESC samplerDesc      = { };
    samplerDesc.AddressU                = getAddressMode(desc.addressModeU);
    samplerDesc.AddressV                = getAddressMode(desc.addressModeV);
    samplerDesc.AddressW                = getAddressMode(desc.addressModeW);
    samplerDesc.BorderColor;
    samplerDesc.ComparisonFunc          = getNativeComparisonFunction(desc.compareOp);
    // For some reason D3D12_FILTER_ANISOTROPIC uses linear filtering, but vulkan allows using both linear and nearest filtering...
    // Might need to keep it consistent between the two. D3D12 literally forces you to use linear filtering, whenever you use anisotropy.
    samplerDesc.Filter                  = (desc.maxAnisotropy > 0.f) 
                                                ? D3D12_FILTER_ANISOTROPIC 
                                                : getFilterMode(desc.minFilter, desc.magFilter, desc.mipMapMode, isCompare);
    samplerDesc.MaxAnisotropy           = desc.maxAnisotropy;
    samplerDesc.MinLOD                  = desc.minLod;
    samplerDesc.MaxLOD                  = desc.maxLod;
    samplerDesc.MipLODBias              = desc.mipLodBias;

    m_hashId = recluseHashFast(&desc, sizeof(SamplerDescription));
    DescriptorHeapAllocationManager* manager = pDevice->getDescriptorHeapManager();
    m_handle = manager->allocateSampler(samplerDesc);
    return RecluseResult_Ok;
}


ResultCode D3D12Sampler::destroy(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);
    DescriptorHeapAllocationManager* manager = pDevice->getDescriptorHeapManager();
    // Need to free descriptor handle!!
    return RecluseResult_NoImpl;
}
} // D3D12
} // Recluse