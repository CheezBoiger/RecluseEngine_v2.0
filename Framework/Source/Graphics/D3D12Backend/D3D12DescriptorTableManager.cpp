//
#include "D3D12DescriptorTableManager.hpp"
#include "D3D12Device.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"

namespace Recluse {


const D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable::invalidGpuAddress = { 0 };
const D3D12_CPU_DESCRIPTOR_HANDLE DescriptorTable::invalidCpuAddress = { 0 };
const F32 DescriptorHeapAllocationManager::kNumDescriptorsPageSize              = 1024.0f;
const F32 DescriptorHeapAllocationManager::kNumSamplerDescriptorsPageSize       = 256.f;


R_INTERNAL CpuHeapType getGpuToCpuHeapTypeMatch(GpuHeapType gpuHeapType)
{
    switch (gpuHeapType)
    {
        case GpuHeapType_CbvSrvUav: return CpuHeapType_CbvSrvUavStaging;
        case GpuHeapType_Sampler: return CpuHeapType_SamplerStaging;
        default: return CpuHeapType_Unknown;
    }
}


R_INTERNAL GpuHeapType getCpuToGpuHeapTypeMatch(CpuHeapType cpuHeapType)
{
    switch (cpuHeapType)
    {
        case CpuHeapType_CbvSrvUavStaging: return GpuHeapType_CbvSrvUav;
        case CpuHeapType_SamplerStaging: return GpuHeapType_Sampler;
        default: return GpuHeapType_Unknown;
    }
}


R_INTERNAL GpuHeapType getGpuHeapTypeFromNative(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    switch (type)
    {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return GpuHeapType_CbvSrvUav;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return GpuHeapType_Sampler;
        default:
            return GpuHeapType_Unknown;
    }
}


R_INTERNAL CpuHeapType getCpuHeapTypeFromNative(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    switch (type)
    {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return CpuHeapType_CbvSrvUavStaging;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            return CpuHeapType_Dsv;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            return CpuHeapType_Rtv;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return CpuHeapType_SamplerStaging;
        default:
            return CpuHeapType_Unknown;
    }
}


R_INTERNAL const char* getCpuHeapTypeName(CpuHeapType type)
{
    switch (type)
    {
        case CpuHeapType_CbvSrvUavStaging:
            return "CbvSrvUav Staging";
        case CpuHeapType_Dsv:
            return "Dsv";
        case CpuHeapType_Rtv:
            return "Rtv";
        case CpuHeapType_SamplerStaging:
            return "Sampler Staging";
        default:
            return "Unknown";
    }
}


R_INTERNAL const char* getGpuHeapTypeName(GpuHeapType type)
{
    switch (type)
    {
        case GpuHeapType_CbvSrvUav:
            return "CbvSrvUav";
        case GpuHeapType_Sampler:
            return "Sampler";
        default:
            return "Unknown";
    }
}


DescriptorHeapAllocation::DescriptorHeapAllocation
        (
            DescriptorHeap* pDescriptorHeap, 
            const DescriptorTable& descriptorTable,
            U32 descriptorSize,
            U32 heapId
        )
    : m_descriptorTable(descriptorTable)
    , m_pDescriptorHeap(pDescriptorHeap)
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
    D3D12_GPU_DESCRIPTOR_HANDLE baseHandle  = m_descriptorTable.baseGpuDescriptorHandle;
    D3D12_GPU_VIRTUAL_ADDRESS baseAddress   = baseHandle.ptr;
    const U64 offset64                      = static_cast<U64>(entryOffset);
    const U64 incSz64                       = static_cast<U64>(m_descIncSz);

    D3D12_GPU_VIRTUAL_ADDRESS address   = baseAddress + offset64 * incSz64;  
    D3D12_GPU_DESCRIPTOR_HANDLE handle  = { address };

    return handle;
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::getCpuDescriptor(U32 entryOffset) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE baseHandle  = m_descriptorTable.baseCpuDescriptorHandle;
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
    m_currentTotalEntries = 0;
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


DescriptorTable CpuDescriptorHeap::allocate(U32 numberDescriptors)
{
    DescriptorTable allocation;

    R_ASSERT_FORMAT
        (
            hasAvailableSpaceForRequest(numberDescriptors),
            "Descriptor allocation request spills over maximum descriptors in current frame. Request=%d, Max=%d",
            numberDescriptors, m_heapDesc.NumDescriptors
        );

    ResultCode err         = RecluseResult_Ok;

    UPtr allocatedAddress = m_allocator->allocate(numberDescriptors * m_descriptorSize, m_descriptorSize);
    err = m_allocator->getLastError();
    if (err == RecluseResult_Ok)
    {
        allocation.baseCpuDescriptorHandle  = { allocatedAddress };
        allocation.numberDescriptors        = numberDescriptors;
        m_currentTotalEntries += numberDescriptors;
    }
    return allocation;
}


DescriptorTable ShaderVisibleDescriptorHeap::allocate(U32 numberDescriptors)
{
    DescriptorTable allocation;
    R_ASSERT_FORMAT
        (
            hasAvailableSpaceForRequest(numberDescriptors), 
            "Descriptor allocation request spills over maximum descriptors in current frame. Request=%d, Max=%d", 
            numberDescriptors, m_heapDesc.NumDescriptors
        );

    ResultCode err         = RecluseResult_Ok;

    UPtr address = m_allocator->allocate(numberDescriptors * m_descriptorSize, m_descriptorSize);
    err = m_allocator->getLastError();    
    if (err == RecluseResult_Ok)
    {
        // Obtain the offset, to be used to map to the base gpu handle.
        UPtr offset                                 = (address - m_baseCpuHandle.ptr) / m_descriptorSize;
        D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = (m_baseGpuHandle.ptr + offset * m_descriptorSize); 
        allocation.baseCpuDescriptorHandle          = { address };
        allocation.baseGpuDescriptorHandle          = { gpuVirtualAddress };
        allocation.numberDescriptors                = numberDescriptors;
        m_currentTotalEntries += numberDescriptors;
    }

    return allocation;
}


SmartPtr<Allocator> ShaderVisibleDescriptorHeap::makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes)
{
    // For gpu descriptor heap, we are instead still using the cpu handle, because we can't visibly assign 
    // the gpu address.
    SmartPtr<Allocator> allocator = new LinearAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE baseAddress = pHeap->GetCPUDescriptorHandleForHeapStart();
    allocator->initialize(baseAddress.ptr, numDescriptors * descriptorSizeBytes);
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
            cpuType = CpuHeapType_Dsv;
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            cpuType = CpuHeapType_CbvSrvUavStaging;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            cpuType = CpuHeapType_SamplerStaging;
        default:
            R_WARN(R_CHANNEL_D3D12, "Can not determine the cpu descriptor heap allocation for this request.");
            break;
    }

    R_ASSERT_FORMAT((cpuType != CpuHeapType_Unknown), "Cpu heap type %s used, which is unsupported.", getCpuHeapTypeName(cpuType));
    DescriptorHeapAllocation allocation;
    CpuDescriptorHeap& descriptorHeap = m_cpuHeap[cpuType];

    DescriptorTable table = descriptorHeap.allocate(numberDescriptors);
    allocation = DescriptorHeapAllocation(&descriptorHeap, table, (U32)0, descriptorHeap.getHeapId());
    if (!allocation.isValid())
    {
        R_ERROR(R_CHANNEL_D3D12, "Unable to allocate proper number of descriptors for heap type %s!!", getCpuHeapTypeName(cpuType));
        // TODO: We might want to look into resizing the descriptor heap, incase we have so many descriptors?
    }
    else if (getCpuToGpuHeapTypeMatch(cpuType) != GpuHeapType_Unknown)
    {
        ShaderVisibleDescriptorHeap& gpuHeap = m_gpuHeap[getCpuToGpuHeapTypeMatch(cpuType)];
        DescriptorTable visibleTable = gpuHeap.allocate(numberDescriptors);
        allocation.assignGpuDescriptor(visibleTable.baseGpuDescriptorHandle);
    }
    return allocation;
}


void DescriptorHeapInstance::free(const DescriptorHeapAllocation& alloc)
{
    U32 descriptorSizeBytes                         = alloc.getDescriptorSize();
    D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType   = alloc.getDescriptorType();
    CpuHeapType cpuHeapType                         = getCpuHeapTypeFromNative(descriptorHeapType);
    GpuHeapType gpuHeapType                         = getGpuHeapTypeFromNative(descriptorHeapType);
    CpuDescriptorHeap& cpuDescriptorHeap            = m_cpuHeap[cpuHeapType];
    cpuDescriptorHeap.free(alloc.getDescriptorTableCopy());
}


ResultCode DescriptorHeapAllocationManager::initialize(ID3D12Device* pDevice, const DescriptorCoreSize& descriptorSizes, U32 bufferCount)
{
    return RecluseResult_Ok;
}


void DescriptorHeapInstance::upload(ID3D12Device* pDevice)
{
    for (U32 i = 0; i < GpuHeapType_DescriptorHeapCount; ++i)
    {
        ShaderVisibleDescriptorHeap& gpuHeap                      = m_gpuHeap[i];
        CpuDescriptorHeap& cpuHeap                      = m_cpuHeap[getGpuToCpuHeapTypeMatch(static_cast<GpuHeapType>(i))];
        U32 gpuHeapDescriptorCount                      = gpuHeap.getTotalDescriptorCount();
        U32 cpuHeapDescriptorCount                      = cpuHeap.getTotalDescriptorCount();
        U32 currentEntries                              = cpuHeap.getAllocatedEntries();
        D3D12_CPU_DESCRIPTOR_HANDLE visibleHeapStart    = gpuHeap.getBaseCpuHandle();
        D3D12_CPU_DESCRIPTOR_HANDLE stagingHeapStart    = cpuHeap.getBaseCpuHandle();
        
        R_ASSERT_FORMAT
            (
                (gpuHeapDescriptorCount >= cpuHeapDescriptorCount), 
                "Shader Visible heap is smaller than the Staging descriptor heap! (Visible=%d, Staging=%d)",
                gpuHeapDescriptorCount, cpuHeapDescriptorCount
            );

        if (currentEntries > 0)
            pDevice->CopyDescriptorsSimple(currentEntries, visibleHeapStart, stagingHeapStart, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            
    }
} 


void DescriptorHeapInstance::reset()
{
    for (U32 i = 0; i < m_cpuHeap.size(); ++i)
    {
        m_cpuHeap[i].reset();
    }

    for (U32 i = 0; i < m_gpuHeap.size(); ++i)
        m_gpuHeap[i].reset();
}


void DescriptorHeapInstance::update(DescriptorHeapUpdateFlags updateFlags)
{
    if (updateFlags & DescriptorHeapUpdateFlag_Reset)
    {
        reset();
    }
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