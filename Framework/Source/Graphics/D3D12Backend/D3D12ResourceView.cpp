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
    ResourceViewDesc resourceDesc = getDesc();

    switch (getDesc().type) 
    {   
        case ResourceViewType_RenderTarget:
        {
            D3D12_RENDER_TARGET_VIEW_DESC desc = { };
            m_rtvDesc = desc;
            break;
        }

        case ResourceViewType_DepthStencil:
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC desc = { };
            m_dsvDesc = desc;
            break;
        }

        case ResourceViewType_ShaderResource:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = { };
            m_srvDesc = desc;
            break;
        }

        case ResourceViewType_UnorderedAccess:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { };
            m_uavDesc = desc;
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