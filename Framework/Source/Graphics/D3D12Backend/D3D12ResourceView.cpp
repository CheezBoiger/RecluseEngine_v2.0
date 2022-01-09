//
#include "D3D12Commons.hpp"
#include "D3D12ResourceView.hpp"
#include "D3D12Device.hpp"
#include "D3D12DescriptorTableManager.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType D3D12Sampler::initialize(D3D12Device* pDevice, const SamplerCreateDesc& desc)
{
    R_ASSERT(pDevice != NULL);

    D3D12_SAMPLER_DESC samplerDesc = { };

    m_samplerCPUAddr = pDevice->createSampler(samplerDesc);

    if (m_samplerCPUAddr.ptr == 0) 
    {
        R_ERR(R_CHANNEL_D3D12, "Failed to alloc sampler from device!");
        return REC_RESULT_FAILED;
    }

    return REC_RESULT_OK;
}


ErrType D3D12Sampler::destroy(D3D12Device* pDevice)
{
    R_ASSERT(pDevice != NULL);
    ID3D12Device* device = pDevice->get();
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse