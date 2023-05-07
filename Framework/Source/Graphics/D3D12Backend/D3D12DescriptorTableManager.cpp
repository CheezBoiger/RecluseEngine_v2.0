//
#include "D3D12DescriptorTableManager.hpp"
#include "D3D12Device.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"

namespace Recluse {


const D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::invalidGpuAddress = { 0 };
const D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::invalidCpuAddress = { 0 };
const F32 DescriptorHeapAllocationManager::kNumDescriptorsPageSize              = 1024.0f;
const F32 DescriptorHeapAllocationManager::kNumSamplerDescriptorsPageSize       = 256.f;


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
    , m_allocator(nullptr)
{
}


SmartPtr<Allocator> DescriptorHeap::makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes)
{
    SmartPtr<Allocator> allocator = makeSmartPtr(new LinearAllocator());
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor = pHeap->GetCPUDescriptorHandleForHeapStart();
    allocator->initialize(descriptor.ptr, numDescriptors * descriptorSizeBytes);
    return allocator;
}


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


void DescriptorHeap::reset()
{
    R_ASSERT(m_allocator);
    m_allocator->reset();
}


ResultCode DescriptorHeap::initialize(ID3D12Device* pDevice, U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
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

    m_allocator = makeAllocator(m_pHeap, numDescriptors, m_descriptorSize);

    return RecluseResult_Ok;
}


ResultCode DescriptorHeap::release(ID3D12Device* pDevice)
{
    if (m_allocator)
    {
        m_allocator->cleanUp();
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

    R_ASSERT_FORMAT
        (
            (m_currentTotalEntries + numberDescriptors) < m_heapDesc.NumDescriptors,
            "Descriptor allocation request spills over maximum descriptors in current frame. Request=%d, Max=%d",
            numberDescriptors, m_heapDesc.NumDescriptors
        );

    Allocation alloc    = { };
    ResultCode err         = RecluseResult_Ok;

    err = m_allocator->allocate(&alloc, numberDescriptors * m_descriptorSize, m_descriptorSize);
    
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

    R_ASSERT_FORMAT
        (
            (m_currentTotalEntries + numberDescriptors) < m_heapDesc.NumDescriptors, 
            "Descriptor allocation request spills over maximum descriptors in current frame. Request=%d, Max=%d", 
            numberDescriptors, m_heapDesc.NumDescriptors
        );

    Allocation alloc    = { };
    ResultCode err         = RecluseResult_Ok;

    err = m_allocator->allocate(&alloc, numberDescriptors * m_descriptorSize, m_descriptorSize);
    
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
    SmartPtr<Allocator> allocator = new LinearAllocator();
    D3D12_GPU_VIRTUAL_ADDRESS baseAddress = pHeap->GetGPUDescriptorHandleForHeapStart().ptr;
    allocator->initialize(baseAddress, numDescriptors * descriptorSizeBytes);
    return allocator;
}


DescriptorHeapAllocation DescriptorHeapInstance::allocate(ID3D12Device* pDevice, U32 numberDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
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
    return allocation;
}


void DescriptorHeapInstance::free(const DescriptorHeapAllocation& alloc)
{
    R_NO_IMPL();
}


ResultCode DescriptorHeapAllocationManager::initialize(ID3D12Device* pDevice, const DescriptorCoreSize& descriptorSizes, U32 bufferCount)
{
    return RecluseResult_Ok;
}


void DescriptorHeapInstance::upload(ID3D12Device* pDevice)
{
} 


void DescriptorHeapInstance::update(U32 index, DescriptorHeapUpdateFlags updateFlags)
{
}


namespace Binder {


ID3D12RootSignature* makeRootSignature()
{
    // D3D12SerializeRootSignature();
    D3D12_ROOT_SIGNATURE_DESC desc = { };

    ID3D12GraphicsCommandList* list;

    return nullptr;
}
} // Binder
} // Recluse