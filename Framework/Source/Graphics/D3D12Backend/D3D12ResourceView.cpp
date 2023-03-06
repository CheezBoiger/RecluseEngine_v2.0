//
#include "D3D12Commons.hpp"
#include "D3D12ResourceView.hpp"
#include "D3D12Device.hpp"
#include "D3D12DescriptorTableManager.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType D3D12GraphicsResourceView::initialize(D3D12Device* pDevice)
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
            break;
        }

        case ResourceViewType_DepthStencil:
        {
            dsvDescription.ViewDimension = getDsvDimension(resourceViewDescription.dimension);
            break;
        }

        case ResourceViewType_ShaderResource:
        {
            srvDescription.ViewDimension = getSrvDimension(resourceViewDescription.dimension);
            break;
        }

        case ResourceViewType_UnorderedAccess:
        {
            uavDescription.ViewDimension = getUavDimension(resourceViewDescription.dimension);
            break;
        }

        default:
            break;
    }

    return RecluseResult_Ok;
}


ErrType D3D12Sampler::initialize(D3D12Device* pDevice, const SamplerCreateDesc& desc)
{
    R_ASSERT(pDevice != NULL);

    D3D12_SAMPLER_DESC samplerDesc = { };

    m_samplerDesc = samplerDesc;

    return RecluseResult_NoImpl;
}


ErrType D3D12Sampler::destroy(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);
    ID3D12Device* device = pDevice->get();
    return RecluseResult_NoImpl;
}
} // Recluse