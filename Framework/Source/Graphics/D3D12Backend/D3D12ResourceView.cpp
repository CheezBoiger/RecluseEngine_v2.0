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


ResultCode D3D12Sampler::initialize(D3D12Device* pDevice, const SamplerDescription& desc)
{
    R_ASSERT(pDevice != NULL);

    D3D12_SAMPLER_DESC samplerDesc = { };

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