//
#include "D3D12DescriptorTableManager.hpp"
#include "D3D12Device.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"

namespace Recluse {


const D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable::invalidGpuAddress                        = { 0 };
const D3D12_CPU_DESCRIPTOR_HANDLE DescriptorTable::invalidCpuAddress                        = { 0 };
const F32 DescriptorHeapAllocationManager::kNumDescriptorsPageSize                          = 1024.0f;
const F32 DescriptorHeapAllocationManager::kNumSamplerDescriptorsPageSize                   = 256.f;
const U32 DescriptorHeapAllocationManager::kMaxedReservedShaderVisibleInstances             = 16;
const U32 ShaderVisibleDescriptorHeapInstance::kMaxShaderVisibleHeapDescriptorSize          = 4096u;
const U32 ShaderVisibleDescriptorHeapInstance::kMaxShaderVisibleHeapSamplerDescriptorSize   = 2048u;


R_INTERNAL CpuHeapType getGpuToCpuHeapTypeMatch(GpuHeapType gpuHeapType)
{
    switch (gpuHeapType)
    {
        case GpuHeapType_CbvSrvUav: return CpuHeapType_CbvSrvUav;
        case GpuHeapType_Sampler: return CpuHeapType_Sampler;
        default: return CpuHeapType_Unknown;
    }
}


R_INTERNAL GpuHeapType getCpuToGpuHeapTypeMatch(CpuHeapType cpuHeapType)
{
    switch (cpuHeapType)
    {
        case CpuHeapType_CbvSrvUav: return GpuHeapType_CbvSrvUav;
        case CpuHeapType_Sampler: return GpuHeapType_Sampler;
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


R_INTERNAL
D3D12_DESCRIPTOR_HEAP_TYPE getNativeFromCpuHeapType(CpuHeapType type)
{
    switch (type)
    {
        default:
        case CpuHeapType_CbvSrvUav:
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        case CpuHeapType_Dsv:
            return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        case CpuHeapType_Rtv:
            return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        case CpuHeapType_Sampler:
            return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    }
}


R_INTERNAL
D3D12_DESCRIPTOR_HEAP_TYPE getNativeFromGpuHeapType(GpuHeapType type)
{
    switch (type)
    {
        default:
        case GpuHeapType_CbvSrvUav:
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        case GpuHeapType_Sampler:
            return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    }
}


R_INTERNAL CpuHeapType getCpuHeapTypeFromNative(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    switch (type)
    {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return CpuHeapType_CbvSrvUav;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            return CpuHeapType_Dsv;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            return CpuHeapType_Rtv;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return CpuHeapType_Sampler;
        default:
            return CpuHeapType_Unknown;
    }
}


R_INTERNAL const char* getCpuHeapTypeName(CpuHeapType type)
{
    switch (type)
    {
        case CpuHeapType_CbvSrvUav:
            return "CbvSrvUav";
        case CpuHeapType_Dsv:
            return "Dsv";
        case CpuHeapType_Rtv:
            return "Rtv";
        case CpuHeapType_Sampler:
            return "Sampler";
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


DescriptorHeap::DescriptorHeap()
    : m_pHeap(nullptr)
    , m_allocator(nullptr)
    , m_baseCpuHandle(DescriptorTable::invalidCpuAddress)
    , m_baseGpuHandle(DescriptorTable::invalidGpuAddress)
{
}


SmartPtr<Allocator> DescriptorHeap::makeAllocator(ID3D12DescriptorHeap* pHeap, U64 numDescriptors, U64 descriptorSizeBytes)
{
    SmartPtr<Allocator> allocator = new LinearAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor = pHeap->GetCPUDescriptorHandleForHeapStart();
    allocator->initialize(descriptor.ptr, numDescriptors * descriptorSizeBytes);
    return allocator;
}


void DescriptorHeap::reset()
{
    R_ASSERT(m_allocator);
    m_allocator->reset();
    m_currentTotalEntries = 0;
    m_freeAllocations.clear();
}


Bool DescriptorHeap::contains(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    UINT64 beginAddress = m_baseCpuHandle.ptr;
    UINT64 endAddress = m_baseCpuHandle.ptr + m_heapDesc.NumDescriptors * m_descriptorSize;
    UINT64 address = handle.ptr;
    return ((address >= beginAddress) && (address <= endAddress));
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
    if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        m_baseGpuHandle = m_pHeap->GetGPUDescriptorHandleForHeapStart();

    m_allocator = makeAllocator(m_pHeap, numDescriptors, m_descriptorSize);

    return RecluseResult_Ok;
}


ResultCode DescriptorHeap::release()
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
    m_freeAllocations.clear();
    return RecluseResult_Ok;
}


CpuDescriptorTable CpuDescriptorHeap::allocate(U32 numberDescriptors)
{
    CpuDescriptorTable allocation;

    R_ASSERT_FORMAT
        (
            hasAvailableSpaceForRequest(numberDescriptors),
            "Descriptor allocation request spills over maximum descriptors in current frame. Request=%d, Max=%d",
            numberDescriptors, m_heapDesc.NumDescriptors
        );

    ResultCode err         = RecluseResult_Ok;

    UPtr allocatedAddress = m_allocator->allocate(numberDescriptors * m_descriptorSize, 0);
    err = m_allocator->getLastError();
    if (err == RecluseResult_Ok)
    {
        allocation.baseCpuDescriptorHandle  = { allocatedAddress };
        allocation.numberDescriptors        = numberDescriptors;
        allocation.descriptorAtomSize       = m_descriptorSize;
        m_currentTotalEntries += numberDescriptors;
    }
    return allocation;
}


void CpuDescriptorHeap::free(const CpuDescriptorTable& descriptorTable)
{
    m_freeAllocations.push_back(descriptorTable);
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


void ShaderVisibleDescriptorHeapInstance::release()
{
    for (U32 i = 0; i < m_gpuHeap.size(); ++i)
    {
        m_gpuHeap[i].release();
    }
}


ResultCode DescriptorHeapAllocationManager::initialize(ID3D12Device* pDevice, const DescriptorCoreSize& descriptorSizes, U32 bufferCount)
{
    m_pDevice = pDevice;
    m_shaderVisibleDescriptorHeapInstances.reserve(kMaxedReservedShaderVisibleInstances);
    resizeShaderVisibleHeapInstances(bufferCount);

    // initialize a null descriptor for each cpu heap type.
    {
        D3D12_RENDER_TARGET_VIEW_DESC desc  = { };
        desc.ViewDimension                  = D3D12_RTV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement            = 0;
        desc.Buffer.NumElements             = 0;
        desc.Format                         = DXGI_FORMAT_R32_FLOAT;
        m_nullRtvDescriptor = allocateRenderTargetView(nullptr, desc);
    }
    return RecluseResult_Ok;
}


void DescriptorHeapAllocationManager::resizeShaderVisibleHeapInstances(U32 bufferIndex)
{
    U32 currentInstancesSize = m_shaderVisibleDescriptorHeapInstances.size();

    if (bufferIndex == currentInstancesSize)
        return;
    if (bufferIndex > kMaxedReservedShaderVisibleInstances)
        return;

    if (bufferIndex > currentInstancesSize)
    {
        for (U32 i = currentInstancesSize; i < bufferIndex; ++i)
        {
            m_shaderVisibleDescriptorHeapInstances.push_back(ShaderVisibleDescriptorHeapInstance());
            m_shaderVisibleDescriptorHeapInstances[i].initialize(m_pDevice);
        }
    }
    else
    {
        for (I32 i = (I32)currentInstancesSize - 1; i >= (I32)bufferIndex; --i)
        {
            m_shaderVisibleDescriptorHeapInstances[i].release();
            m_shaderVisibleDescriptorHeapInstances.pop_back();
        } 
    }
}


ShaderVisibleDescriptorTable ShaderVisibleDescriptorHeapInstance::upload(ID3D12Device* pDevice, GpuHeapType type, const CpuDescriptorTable& table)
{
    ShaderVisibleDescriptorHeap& heap = m_gpuHeap[type];
    ID3D12DescriptorHeap* pHeap = heap.getNative();
    
    const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = heap.getBaseCpuHandle();
    const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = heap.getBaseGpuHandle();
    UINT64& offset = m_gpuOffsets[type];
    D3D12_CPU_DESCRIPTOR_HANDLE offsetHandle = { cpuHandle.ptr + offset * heap.getDescriptorSize() };
    pDevice->CopyDescriptorsSimple(table.numberDescriptors, offsetHandle,  table.baseCpuDescriptorHandle, heap.getDesc().Type);
    D3D12_GPU_DESCRIPTOR_HANDLE shaderVisibleHandle = { gpuHandle.ptr + offset * heap.getDescriptorSize() };
    ShaderVisibleDescriptorTable shaderVisibleTable = ShaderVisibleDescriptorTable(shaderVisibleHandle, table.numberDescriptors);
    offset += offset + table.numberDescriptors;
    return shaderVisibleTable;
}


void ShaderVisibleDescriptorHeapInstance::initialize(ID3D12Device* pDevice)
{
    m_gpuHeap[GpuHeapType_CbvSrvUav].initialize(pDevice, 0, kMaxShaderVisibleHeapDescriptorSize, getNativeFromGpuHeapType(GpuHeapType_CbvSrvUav));
    m_gpuHeap[GpuHeapType_Sampler].initialize(pDevice, 0, kMaxShaderVisibleHeapSamplerDescriptorSize, getNativeFromGpuHeapType(GpuHeapType_Sampler));
}


void ShaderVisibleDescriptorHeapInstance::update(DescriptorHeapUpdateFlags updateFlags)
{
    if (updateFlags & DescriptorHeapUpdateFlag_Reset)
    {
        for (U32 i = 0; i < m_gpuOffsets.size(); ++i)
        {
            m_gpuOffsets[i] = 0;
        }
    }
}


void DescriptorHeapAllocationManager::resetTableHeaps()
{
    for (auto iter : m_cpuDescriptorTableHeaps)
    {
        for (auto heap : iter.second)
        {
            heap.reset();
        }
    }

    m_currentTableHeapIndex = 0;
}


CpuDescriptorTable DescriptorHeapAllocationManager::copyDescriptorsToTable(CpuHeapType type, D3D12_CPU_DESCRIPTOR_HANDLE* handles, U32 descriptorCount)
{
    CpuDescriptorHeap* heap = getCurrentTableHeap(type, descriptorCount);
    CpuDescriptorTable table = heap->allocate(descriptorCount);
    D3D12_CPU_DESCRIPTOR_HANDLE destHandleStart = table.baseCpuDescriptorHandle;
    UINT destDescriptorRange = table.numberDescriptors;
    std::vector<UINT> srcRangeSizes(descriptorCount);
    for (U32 i = 0; i < descriptorCount; ++i)
        srcRangeSizes[i] = 1;
    m_pDevice->CopyDescriptors(1, &destHandleStart, &destDescriptorRange, descriptorCount, handles, srcRangeSizes.data(), getNativeFromCpuHeapType(type));
    return table;
}


CpuDescriptorHeap* DescriptorHeapAllocationManager::getCurrentTableHeap(CpuHeapType type, U32 descriptorCount)
{
    CpuDescriptorHeap* heap = nullptr;
    if (m_cpuDescriptorTableHeaps[type].empty())
    {
        m_currentTableHeapIndex = 0;
    }
    else
    {
        CpuDescriptorHeap* tempHeap = &m_cpuDescriptorTableHeaps[type][m_currentTableHeapIndex];
        if (tempHeap->hasAvailableSpaceForRequest(descriptorCount))
        {
            heap = tempHeap;
        }
        else
        {
            ++m_currentTableHeapIndex;   
        }
    }

    if (!heap)
    {
        heap = createNewCpuDescriptorTableHeap(type);
    }

    return heap;
}


CpuDescriptorHeap* DescriptorHeapAllocationManager::createNewCpuDescriptorTableHeap(CpuHeapType type)
{
    auto& heapVector = m_cpuDescriptorTableHeaps[type];
    heapVector.push_back(CpuDescriptorHeap());
    auto& newHeap = heapVector.back();
    newHeap.initialize(m_pDevice, 0, kNumDescriptorsPageSize, getNativeFromCpuHeapType(type));
    return &newHeap;
}


CpuDescriptorHeap* DescriptorHeapAllocationManager::getCurrentHeap(CpuHeapType type, U32 descriptorCount)
{
    CpuDescriptorHeap* heap = nullptr;
    if (m_cpuDescriptorHeaps[type].empty())
    {
        m_currentHeapIndex = 0;
    }
    else
    {
        CpuDescriptorHeap* tempHeap = &m_cpuDescriptorHeaps[type][m_currentHeapIndex];
        if (tempHeap->hasAvailableSpaceForRequest(descriptorCount))
        {
            heap = tempHeap;
        }
        else
        {
            ++m_currentHeapIndex;   
        }
    }

    if (!heap)
    {
        heap = createNewCpuDescriptorHeap(type);
    }

    return heap;
}


CpuDescriptorHeap* DescriptorHeapAllocationManager::createNewCpuDescriptorHeap(CpuHeapType type)
{
    auto& heapVector = m_cpuDescriptorHeaps[type];
    heapVector.push_back(CpuDescriptorHeap());
    auto& newHeap = heapVector.back();
    newHeap.initialize(m_pDevice, 0, kNumDescriptorsPageSize, getNativeFromCpuHeapType(type));
    return &newHeap;
}


CpuDescriptorTable DescriptorHeapAllocationManager::internalAllocate(CpuHeapType heapType, U32 numDescriptors)
{
    CpuDescriptorHeap* heap = getCurrentHeap(heapType, numDescriptors);
    CpuDescriptorTable handle = heap->allocate(numDescriptors);
    return handle;   
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocationManager::allocateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
{
    CpuDescriptorTable handle = internalAllocate(CpuHeapType_Rtv, 1);
    m_pDevice->CreateRenderTargetView(pResource, &desc, handle.baseCpuDescriptorHandle);
    return handle.baseCpuDescriptorHandle;
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocationManager::allocateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
    CpuDescriptorTable handle = internalAllocate(CpuHeapType_CbvSrvUav, 1);
    m_pDevice->CreateShaderResourceView(pResource, &desc, handle.baseCpuDescriptorHandle);
    return handle.baseCpuDescriptorHandle;
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocationManager::allocateUnorderedAccessView(ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    CpuDescriptorTable handle = internalAllocate(CpuHeapType_CbvSrvUav, 1);
    m_pDevice->CreateUnorderedAccessView(pResource, nullptr, &desc, handle.baseCpuDescriptorHandle);
    return handle.baseCpuDescriptorHandle;
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocationManager::allocateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
{
    CpuDescriptorTable handle = internalAllocate(CpuHeapType_Dsv, 1);
    m_pDevice->CreateDepthStencilView(pResource, &desc, handle.baseCpuDescriptorHandle);
    return handle.baseCpuDescriptorHandle;
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocationManager::allocateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
{
    CpuDescriptorTable handle = internalAllocate(CpuHeapType_CbvSrvUav, 1);
    m_pDevice->CreateConstantBufferView(&desc, handle.baseCpuDescriptorHandle);
    return handle.baseCpuDescriptorHandle;
}


ResultCode DescriptorHeapAllocationManager::freeRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    return internalFree(handle, CpuHeapType_Rtv);
}


ResultCode DescriptorHeapAllocationManager::freeShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    return internalFree(handle, CpuHeapType_CbvSrvUav);
}


ResultCode DescriptorHeapAllocationManager::freeConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    return internalFree(handle, CpuHeapType_CbvSrvUav);
}


ResultCode DescriptorHeapAllocationManager::freeUnorderedAccessView(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    return internalFree(handle, CpuHeapType_CbvSrvUav);
}


ResultCode DescriptorHeapAllocationManager::freeDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    return internalFree(handle, CpuHeapType_Dsv);
}


ResultCode DescriptorHeapAllocationManager::freeDescriptorTable(CpuHeapType heapType, const CpuDescriptorTable& table)
{
    if (table.baseCpuDescriptorHandle.ptr == DescriptorTable::invalidCpuAddress.ptr)
    {
        return RecluseResult_InvalidArgs;
    }
    for (U32 i = 0; i < m_cpuDescriptorTableHeaps[heapType].size(); ++i)
    {
        CpuDescriptorHeap& heap = m_cpuDescriptorTableHeaps[heapType][i];
        if (heap.contains(table.baseCpuDescriptorHandle))
        {
            heap.free(table);
            break;
        }
    }
    return RecluseResult_Ok;
}


ResultCode DescriptorHeapAllocationManager::internalFree(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, CpuHeapType heapType)
{
    if (descriptor.ptr == DescriptorTable::invalidCpuAddress.ptr)
        return RecluseResult_InvalidArgs;
    for (U32 i = 0; i < m_cpuDescriptorHeaps[heapType].size(); ++i)
    {
        CpuDescriptorHeap& heap = m_cpuDescriptorHeaps[heapType][i];
        if (heap.contains(descriptor))
        {
            CpuDescriptorTable makeShiftTable       = { };
            makeShiftTable.descriptorAtomSize       = heap.getDescriptorSize();
            makeShiftTable.baseCpuDescriptorHandle  = descriptor;
            makeShiftTable.numberDescriptors        = 1;
            heap.free(makeShiftTable);
            break;
        }
    }
    return RecluseResult_Ok;
}


ResultCode DescriptorHeapAllocationManager::release()
{
    resizeShaderVisibleHeapInstances(0);
    for (auto iter : m_cpuDescriptorHeaps)
    {
        for (auto heap : iter.second)
        {
            heap.release();
        }
    }

    for (auto iter : m_cpuDescriptorTableHeaps)
    {
        for (auto heap : iter.second)
        {
            heap.release();
        }
    }
    m_currentTableHeapIndex = 0;
    m_currentHeapIndex = 0;
    return RecluseResult_Ok;
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