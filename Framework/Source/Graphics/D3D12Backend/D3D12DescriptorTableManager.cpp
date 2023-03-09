//
#include "D3D12DescriptorTableManager.hpp"
#include "D3D12Device.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"

namespace Recluse {


const D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::invalidGpuAddress = { 0 };
const D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::invalidCpuAddress = { 0 };
const F32 DescriptorHeapAllocationManager::kNumDescriptorsPageSize              = 1024.0f;
const F32 DescriptorHeapAllocationManager::kNumSamplerDescriptorsPageSize       = 128.f;


DescriptorHeapAllocation::DescriptorHeapAllocation
        (
            DescriptorHeap* pDescriptorHeap, 
            D3D12_CPU_DESCRIPTOR_HANDLE baseCpuHandle, 
            D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle, 
            U32 totalHandles,
            U32 descriptorSize,
            U32 heapId
        )
    : m_baseCpuDescriptorHandle(baseCpuHandle)
    , m_baseGpuDescriptorHandle(baseGpuHandle)
    , m_pDescriptorHeap(pDescriptorHeap)
    , m_totalHandles(totalHandles)
    , m_descIncSz(descriptorSize)
    , m_heapId(heapId)
{
}


DescriptorHeap::DescriptorHeap()
    : m_pHeap(nullptr)
    , m_pDescriptorHeapAllocator(nullptr)
{
}


SmartPtr<Allocator> DescriptorHeap::makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes)
{
    SmartPtr<Allocator> allocator = makeSmartPtr(new LinearAllocator());
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor = pHeap->GetCPUDescriptorHandleForHeapStart();
    allocator->initialize(descriptor.ptr, numDescriptors * descriptorSizeBytes);
    return allocator;
}

//
//
//ErrType D3D12DescriptorHeapAllocation::initialize(D3D12_DESCRIPTOR_HEAP_TYPE type, U32 entries)
//{
//    R_ASSERT(m_pDevice != NULL);
//
//    HRESULT result                      = S_OK;
//    ID3D12Device* pDevice               = m_pDevice->get();
//    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = { };
//    heapDesc.NodeMask                   = 0;
//    heapDesc.NumDescriptors             = entries;
//    heapDesc.Type                       = type;
//
//    heapDesc.Flags  = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//
//    // We want to make our descriptor heaps visible to the shaders if they are CBV/SRV/UAV or Samplers.
//    if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
//    {
//        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//    }
//
//    result          = pDevice->CreateDescriptorHeap
//                                    (
//                                        &heapDesc, 
//                                        __uuidof(ID3D12DescriptorHeap),
//                                        (void**)&m_pDescriptorHeap
//                                    );
//    if (FAILED(result)) 
//    {
//        R_ERR(R_CHANNEL_D3D12, "Failed to create descriptor heap.");
//        return R_RESULT_FAILED;
//    }
//
//    m_descHeapType  = type;
//    m_descIncSz     = static_cast<U64>(pDevice->GetDescriptorHandleIncrementSize(type));
//    m_maxEntries    = static_cast<U64>(entries);
//
//    // Ensure we dont obtain the gpu handle on non-shader visible descriptor heaps.
//    if (type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
//    {
//        m_firstGpuDescriptorHandle = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
//    }
//    else
//    {
//        m_firstCpuDescriptorHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//    }
//
//    return R_RESULT_OK;
//}
//
//
//ErrType D3D12DescriptorHeapAllocation::destroy()
//{
//    R_ASSERT(m_pDevice != NULL);
//
//    if (m_pDescriptorHeap) 
//    {
//        m_pDescriptorHeap->Release();
//        m_pDescriptorHeap = nullptr;
//    }
//
//    return R_RESULT_OK;
//}

//
//ErrType D3D12DescriptorHeap::upload(U32 startEntry, U32 endEntry)
//{
//    R_ASSERT(m_pDevice != NULL);
//
//    ID3D12Device* pDevice                       = m_pDevice->get();
//    U32 totalEntries                            = endEntry - startEntry;
//    U64 endEntry64                              = static_cast<U64>(endEntry);
//    U64 startEntry64                            = static_cast<U64>(startEntry);
//    U64 szDesc                                  = m_descIncSz;
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuSrcHandle    = { m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + startEntry64 * szDesc };
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { 0 };
//
//    if (m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) 
//    {
//        cpuDstHandle = { m_pGPUDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + endEntry64 * szDesc };
//    } 
//    else 
//    {
//        cpuDstHandle = { m_pCPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + endEntry64 * szDesc };
//    }
//
//    pDevice->CopyDescriptorsSimple(totalEntries,
//            cpuDstHandle, cpuSrcHandle, m_descHeapType);
//
//    return R_RESULT_OK;
//}

//
//D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapAllocation::createRenderTargetView(
//    U32 entryOffset, const D3D12_RENDER_TARGET_VIEW_DESC& desc,
//    ID3D12Resource* pResource)
//{
//    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//
//    ID3D12Device* pDevice   = m_pDevice->get();
//    U64 offset64            = static_cast<U64>(entryOffset);
//    U64 incSz64             = static_cast<U64>(m_descIncSz);
//
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle   = m_firstCpuDescriptorHandle;
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { cpuBaseHandle.ptr + offset64 * incSz64 };
//    
//    pDevice->CreateRenderTargetView(pResource, &desc, cpuDstHandle);
//
//    return cpuDstHandle;
//}
//
//
//D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapAllocation::createDepthStencilView(
//    U32 entryOffset, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc,
//    ID3D12Resource* pResource)
//{
//    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//
//    ID3D12Device* pDevice   = m_pDevice->get();
//    U64 offset64            = static_cast<U64>(entryOffset);
//    U64 incSz64             = static_cast<U64>(m_descIncSz);
//
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle = m_firstCpuDescriptorHandle;
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle = { cpuBaseHandle.ptr + offset64 * incSz64 };
//    
//    pDevice->CreateDepthStencilView(pResource, &desc, cpuDstHandle);
//
//    return cpuDstHandle;
//}
//
//
//D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapAllocation::createShaderResourceView(
//    U32 entryOffset, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
//    ID3D12Resource* pResource)
//{
//    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//    
//    ID3D12Device* pDevice   = m_pDevice->get();
//    U64 offset64            = static_cast<U64>(entryOffset);
//    U64 incSz64             = static_cast<U64>(m_descIncSz);
//    
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle   = m_firstCpuDescriptorHandle;
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { cpuBaseHandle.ptr + offset64 * incSz64 };
//    
//    pDevice->CreateShaderResourceView(pResource, &desc, cpuDstHandle);
//    
//    return cpuDstHandle;
//}
//
//
//D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapAllocation::createUnorderedAccessView(
//    U32 entryOffset, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
//    ID3D12Resource* pResource)
//{
//    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//    ID3D12Device* pDevice   = m_pDevice->get();
//    U64 offset64            = static_cast<U64>(entryOffset);
//    U64 incSz64             = static_cast<U64>(m_descIncSz);
//    
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle   = m_firstCpuDescriptorHandle;
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle    = { cpuBaseHandle.ptr + offset64 * incSz64 };
//    
//    pDevice->CreateUnorderedAccessView(pResource, nullptr, &desc, cpuDstHandle);
//
//    return cpuDstHandle;
//}


D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::getGpuDescriptor(U32 entryOffset) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE baseHandle  = m_baseGpuDescriptorHandle;
    D3D12_GPU_VIRTUAL_ADDRESS baseAddress   = baseHandle.ptr;
    const U64 offset64                      = static_cast<U64>(entryOffset);
    const U64 incSz64                       = static_cast<U64>(m_descIncSz);

    D3D12_GPU_VIRTUAL_ADDRESS address   = baseAddress + offset64 * incSz64;  
    D3D12_GPU_DESCRIPTOR_HANDLE handle  = { address };

    return handle;
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::getCpuDescriptor(U32 entryOffset) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE baseHandle  = m_baseCpuDescriptorHandle;
    SIZE_T baseHandlePtr                    = baseHandle.ptr;
    const U64 offset                        = static_cast<U64>(entryOffset);
    const U64 incSz                         = static_cast<U64>(m_descIncSz);

    SIZE_T address                      = baseHandlePtr + offset * incSz;
    D3D12_CPU_DESCRIPTOR_HANDLE handle  = { address };

    return handle;
}

//
//D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapAllocation::createSampler(U32 entryOffset,
//    const D3D12_SAMPLER_DESC& desc)
//{
//    R_ASSERT(m_descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
//
//    ID3D12Device* pDevice   = m_pDevice->get();
//
//    const D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle     = m_firstCpuDescriptorHandle;
//    D3D12_CPU_DESCRIPTOR_HANDLE cpuDstHandle            = getCpuHandle(cpuBaseHandle, m_descIncSz, entryOffset);
//
//    pDevice->CreateSampler(&desc, cpuDstHandle);
//
//    return cpuDstHandle;
//}


ErrType DescriptorHeap::initialize(ID3D12Device* pDevice, U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    R_ASSERT(pDevice != NULL);
    D3D12_DESCRIPTOR_HEAP_DESC desc = makeDescriptorHeapDescription(nodeMask, numDescriptors, type);
    HRESULT result = pDevice->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pHeap);

    if (FAILED(result))
    {
        return RecluseResult_Failed;
    }

    m_heapDesc = desc;
    m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(type);
    m_baseCpuHandle = m_pHeap->GetCPUDescriptorHandleForHeapStart();
    m_baseGpuHandle = m_pHeap->GetGPUDescriptorHandleForHeapStart();

    m_pDescriptorHeapAllocator = makeAllocator(m_pHeap, numDescriptors, m_descriptorSize);

    return RecluseResult_Ok;
}


ErrType DescriptorHeap::release(ID3D12Device* pDevice)
{
    if (m_pDescriptorHeapAllocator)
    {
        m_pDescriptorHeapAllocator->cleanUp();
    }

    if (m_pHeap)
    {
        m_pHeap->Release();
        m_pHeap = nullptr;
    }

    return RecluseResult_Ok;
}


DescriptorHeapAllocation CpuDescriptorHeap::allocate(U32 numberDescriptors)
{
    DescriptorHeapAllocation allocation;

    R_ASSERT((m_currentTotalEntries + numberDescriptors) < m_heapDesc.NumDescriptors);

    Allocation alloc    = { };
    ErrType err         = RecluseResult_Ok;

    err = m_pDescriptorHeapAllocator->allocate(&alloc, numberDescriptors * m_descriptorSize, m_descriptorSize);

    if (err == RecluseResult_Ok)
    {
        allocation = DescriptorHeapAllocation
                        (
                            this, 
                            { alloc.baseAddress }, 
                            DescriptorHeap::invalidGpuAddress, 
                            numberDescriptors, 
                            m_descriptorSize, 
                            m_heapId
                        );
    }
    return allocation;
}


DescriptorHeapAllocation GpuDescriptorHeap::allocate(U32 numberDescriptors)
{
    DescriptorHeapAllocation allocation;

    R_ASSERT((m_currentTotalEntries + numberDescriptors) < m_heapDesc.NumDescriptors);

    Allocation alloc    = { };
    ErrType err         = RecluseResult_Ok;

    err = m_pDescriptorHeapAllocator->allocate(&alloc, numberDescriptors * m_descriptorSize, m_descriptorSize);
    
    if (err == RecluseResult_Ok)
    {
        allocation = DescriptorHeapAllocation
                        (
                            this,
                            DescriptorHeap::invalidCpuAddress,
                            { alloc.baseAddress },
                            numberDescriptors,
                            m_descriptorSize,
                            m_heapId
                        );
    }

    return allocation;
}


SmartPtr<Allocator> GpuDescriptorHeap::makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes)
{
    SmartPtr<Allocator> allocator = makeSmartPtr(new LinearAllocator());
    D3D12_GPU_VIRTUAL_ADDRESS baseAddress = pHeap->GetGPUDescriptorHandleForHeapStart().ptr;
    allocator->initialize(baseAddress, numDescriptors * descriptorSizeBytes);
    return allocator;
}


DescriptorHeapAllocation CpuDescriptorHeapManager::allocate(ID3D12Device* pDevice, U32 numberDescriptors)
{
    DescriptorHeapAllocation allocation;

    // Locate a descriptor heap that has the available space for the requested number of descriptors.
    for (U32 i = 0; i < m_cpuHeaps.size(); ++i)
    {
        DescriptorHeap* pHeap = &m_cpuHeaps[i];
        
        if (pHeap->isValid() && pHeap->hasAvailableSpace())
        {
            // Properly allocate the descriptor heap. Return it afterwards.
            allocation = pHeap->allocate(numberDescriptors);
            return allocation;
        }
    }

    // Check if allocation is valid. If not, it means we didn't have enough space anymore in our heaps.
    // We need to create a new one.
    if (!allocation.isValid())
    {
        // Create a new descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC desc = m_cpuHeaps.end()->getDesc();
        m_cpuHeaps.push_back(CpuDescriptorHeap());
        m_cpuHeaps.end()->initialize(pDevice, desc.NodeMask, desc.NumDescriptors, desc.Type);
        allocation = m_cpuHeaps.end()->allocate(numberDescriptors);
    }

    return allocation;
}


DescriptorHeapAllocation DescriptorHeapAllocationManager::allocate(ID3D12Device* pDevice, U32 numberDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    CpuHeapType cpuType = CpuHeapType_Unknown;

    switch (type)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        cpuType = CpuHeapType_Rtv;
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
    default:
        break;
    }

    DescriptorHeapAllocation allocation;
    CpuDescriptorHeapManager* manager = &m_cpuDescriptorHeapManagers[cpuType];

    R_ASSERT(manager != NULL);

    allocation = manager->allocate(pDevice, numberDescriptors);

    if (!allocation.isValid())
    {
        R_ERR("D3D12", "Unable to allocate() given number of descriptors for specified heap type=%d", type);
    }

    return allocation;
}


void DescriptorHeapAllocationManager::free(const DescriptorHeapAllocation& alloc)
{
    R_NO_IMPL();
}


ErrType DescriptorHeapAllocationManager::initialize(ID3D12Device* pDevice, const DescriptorCoreSize& descriptorSizes, U32 bufferCount)
{
    U32 numDescriptors = static_cast<U32>(kNumDescriptorsPageSize * descriptorSizes.rtvDescriptorCountFactor);
    m_cpuDescriptorHeapManagers[CpuHeapType_Rtv].initialize(pDevice, 0, numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    numDescriptors = static_cast<U32>(kNumDescriptorsPageSize * descriptorSizes.dsvDescriptorCountFactor);
    m_cpuDescriptorHeapManagers[CpuHeapType_Dsv].initialize(pDevice, 0, numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    numDescriptors = static_cast<U32>(kNumDescriptorsPageSize * descriptorSizes.cbvSrvUavDescriptorCountFactor);
    m_cpuDescriptorHeapManagers[CpuHeapType_CbvSrvUav].initialize(pDevice, 0, numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    numDescriptors = static_cast<U32>(kNumSamplerDescriptorsPageSize * descriptorSizes.samplerDescriptorCountFactor);
    m_cpuDescriptorHeapManagers[CpuHeapType_Sampler].initialize(pDevice, 0, numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    numDescriptors = static_cast<U32>(kNumDescriptorsPageSize * descriptorSizes.cbvSrvUavDescriptorCountFactor);
    m_gpuHeaps[GpuHeapType_CbvSrvUav].initialize(pDevice, 0, numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    numDescriptors = static_cast<U32>(kNumSamplerDescriptorsPageSize * descriptorSizes.samplerDescriptorCountFactor);
    m_gpuHeaps[GpuHeapType_Sampler].initialize(pDevice, 0, numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    return RecluseResult_Ok;
}


ErrType CpuDescriptorHeapManager::initialize(ID3D12Device* pDevice, U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    if (!m_cpuHeaps.empty())
    {
        return RecluseResult_AlreadyExists;
    }

    m_cpuHeaps.resize(1);
    ErrType result = m_cpuHeaps[0].initialize(pDevice, nodeMask, numDescriptors, type);

    if (result != RecluseResult_Ok)
    {
        
    }

    return result;
}


void DescriptorHeapAllocationManager::requestUpload(const DescriptorHeapAllocation& descriptor, GpuHeapType heapType)
{
} 


namespace Binder {


ID3D12RootSignature* makeRootSignature()
{
    // D3D12SerializeRootSignature();
    return nullptr;
}
} // Binder
} // Recluse