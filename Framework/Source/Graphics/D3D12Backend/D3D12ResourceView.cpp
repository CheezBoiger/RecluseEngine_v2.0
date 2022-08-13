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
        case RESOURCE_VIEW_TYPE_RENDER_TARGET:
        {
            D3D12_RENDER_TARGET_VIEW_DESC desc = { };
            pDevice->createRenderTargetView(desc);
            break;
        }

        case RESOURCE_VIEW_TYPE_DEPTH_STENCIL:
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC desc = { };
            pDevice->createDepthStencilView(desc);
            break;
        }

        case RESOURCE_VIEW_TYPE_SHADER_RESOURCE:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = { };
            pDevice->createShaderResourceView(desc);
            break;
        }

        case RESOURCE_VIEW_TYPE_STORAGE_BUFFER:
        case RESOURCE_VIEW_TYPE_STORAGE_IMAGE:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { };
            pDevice->createUnorderedAccessView(desc);
            break;
        }

        default:
            break;
    }

    return R_RESULT_OK;
}


ErrType D3D12Sampler::initialize(D3D12Device* pDevice, const SamplerCreateDesc& desc)
{
    R_ASSERT(pDevice != NULL);

    D3D12_SAMPLER_DESC samplerDesc = { };

    m_samplerCPUAddr = pDevice->createSampler(samplerDesc);

    if (m_samplerCPUAddr.ptr == 0) 
    {
        R_ERR(R_CHANNEL_D3D12, "Failed to alloc sampler from device!");
        return R_RESULT_FAILED;
    }

    return R_RESULT_OK;
}


ErrType D3D12Sampler::destroy(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);
    ID3D12Device* device = pDevice->get();
    return R_RESULT_NO_IMPL;
}
} // Recluse