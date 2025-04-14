//
#include "D3D12DescriptorTableManager.hpp"
#include "D3D12Device.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"

namespace Recluse {
namespace D3D12 {

const D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable::invalidGpuAddress                        = { 0 };
const D3D12_CPU_DESCRIPTOR_HANDLE DescriptorTable::invalidCpuAddress                        = { 0 };
const F32 DescriptorHeapAllocationManager::kNumDescriptorsPageSize                          = 1024.0f;
const F32 DescriptorHeapAllocationManager::kNumSamplerDescriptorsPageSize                   = 256.f;
const U32 DescriptorHeapAllocationManager::kMaxedReservedShaderVisibleInstances             = 16;

// Max limitations of hardware is standard to d3d12.
// These can be found in this link reference.
//
// https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
const U32 kStandardMaxVisibleDescriptorHeapSize = 1000000u;
R_DECLARE_GLOBAL_U32(g_maxShaderVisibleHeapDescriptorSize, 50000u, "D3D12.MaxShaderVisibleHeapDescriptorSize");
R_DECLARE_GLOBAL_U32(g_maxShaderVisibleHeapSamplerDescriptorSize, 2048u, "D3D12.MaxShaderVisibleHeapSamplerSize");


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


ResultCode DescriptorHeap::initialize(ID3D12Device* pDevice, U32 nodeMask, U32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    R_ASSERT(pDevice != NULL);
    D3D12_DESCRIPTOR_HEAP_DESC desc = makeDescriptorHeapDescription(nodeMask, numDescriptors, type, flags);
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
        D3D12_RENDER_TARGET_VIEW_DESC desc = makeNullRenderTargetViewDescriptor();
        m_nullRtvDescriptor = allocateRenderTargetView(nullptr, desc);
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = makeNullShaderResourceViewDescriptor();
        m_nullSrvDescriptor = allocateShaderResourceView(nullptr, desc);
    }

    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = makeNullUnorderedAccessViewDescriptor();
        m_nullUavDescriptor = allocateUnorderedAccessView(nullptr, desc);
    }

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = makeNullConstantBufferViewDescriptor();
        m_nullCbvDescriptor = allocateConstantBufferView(desc);
    }

    {
        D3D12_SAMPLER_DESC desc = makeNullSamplerDescriptor();
        m_nullSamplerDescriptor = allocateSampler(desc);
    }

    {
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = { };
        desc.Format = DXGI_FORMAT_D32_FLOAT;
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
        desc.Texture1D.MipSlice = 0;
        desc.Flags = D3D12_DSV_FLAG_NONE;
        m_nullDsvDescriptor = allocateDepthStencilView(nullptr, desc);
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
    R_ASSERT_FORMAT
        (
            (heap.getAllocatedEntries() + table.numberDescriptors) <= heap.getTotalDescriptorCount(), 
            "We have exceeded the maximum number of descriptors for our shader visible heap! (Current Allocated Descriptors=%d) (Requested Table size=%d) (Max Heap Size=%d)",
            heap.getAllocatedEntries(), table.numberDescriptors, heap.getTotalDescriptorCount()
        );
    ID3D12DescriptorHeap* pHeap = heap.getNative();
    
    const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = heap.getBaseCpuHandle();
    const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = heap.getBaseGpuHandle();
    UINT64& offset = m_gpuOffsets[type];
    D3D12_CPU_DESCRIPTOR_HANDLE offsetHandle = { cpuHandle.ptr + offset * heap.getDescriptorSize() };
    pDevice->CopyDescriptorsSimple(table.numberDescriptors, offsetHandle,  table.baseCpuDescriptorHandle, heap.getDesc().Type);
    D3D12_GPU_DESCRIPTOR_HANDLE shaderVisibleHandle = { gpuHandle.ptr + offset * heap.getDescriptorSize() };
    ShaderVisibleDescriptorTable shaderVisibleTable = ShaderVisibleDescriptorTable(shaderVisibleHandle, table.numberDescriptors);
    // Update the offset for this visible heap.
    offset = offset + table.numberDescriptors;
    return shaderVisibleTable;
}


void ShaderVisibleDescriptorHeapInstance::initialize(ID3D12Device* pDevice)
{
    R_ASSERT_FORMAT
        (
            g_maxShaderVisibleHeapDescriptorSize < kStandardMaxVisibleDescriptorHeapSize, 
            "Microsoft spec states that our maximum shader visible descriptor size be %d descriptors, but we are specifying %d!", 
            kStandardMaxVisibleDescriptorHeapSize, g_maxShaderVisibleHeapDescriptorSize
        );
    m_gpuHeap[GpuHeapType_CbvSrvUav].initialize(pDevice, 0, g_maxShaderVisibleHeapDescriptorSize, getNativeFromGpuHeapType(GpuHeapType_CbvSrvUav), D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    m_gpuHeap[GpuHeapType_Sampler].initialize(pDevice, 0, g_maxShaderVisibleHeapSamplerDescriptorSize, getNativeFromGpuHeapType(GpuHeapType_Sampler), D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}


void ShaderVisibleDescriptorHeapInstance::update(DescriptorHeapUpdateFlags updateFlags)
{
    if (updateFlags & DescriptorHeapUpdateFlag_Reset)
    {
        for (U32 i = 0; i < m_gpuOffsets.size(); ++i)
        {
            m_gpuOffsets[i] = 0;
        }
        for (U32 i = 0; i < m_gpuHeap.size(); ++i)
        {
            m_gpuHeap[i].reset();
        }
    }
}


void DescriptorHeapAllocationManager::resetCpuTableHeaps()
{
    for (auto& iter : m_cpuDescriptorTableHeaps)
    {
        for (auto& heap : iter.second)
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
        while (m_currentTableHeapIndex < m_cpuDescriptorTableHeaps[type].size())
        {
            CpuDescriptorHeap* tempHeap = &m_cpuDescriptorTableHeaps[type][m_currentTableHeapIndex];
            if (tempHeap->hasAvailableSpaceForRequest(descriptorCount))
            {
                heap = tempHeap;
                break;
            }
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


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocationManager::allocateSampler(const D3D12_SAMPLER_DESC& desc)
{
    CpuDescriptorTable handle = internalAllocate(CpuHeapType_Sampler, 1);
    m_pDevice->CreateSampler(&desc, handle.baseCpuDescriptorHandle);
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


D3D12QueryManager::D3D12QueryManager()
    : m_heap(nullptr)
    , m_scratchBuffer(nullptr)
    , m_currentAvailableIndex(0)
    , m_type(D3D12_QUERY_HEAP_TYPE_OCCLUSION)
{
}


D3D12QueryManager::~D3D12QueryManager()
{
    release();
}


ResultCode D3D12QueryManager::initialize(ID3D12Device* device, UINT nodeMask, D3D12_QUERY_HEAP_TYPE type, U32 maxQueries)
{
    R_ASSERT(device && maxQueries > 0);
    ResultCode result = RecluseResult_Failed;
    if (maxQueries > 0)
    {
        D3D12_QUERY_HEAP_DESC desc = { };
        desc.Count = maxQueries;
        desc.Type = type;
        desc.NodeMask = nodeMask;
        HRESULT hr = device->CreateQueryHeap(&desc, __uuidof(ID3D12QueryHeap), (void**)&m_heap);

        result = SUCCEEDED(hr) ? RecluseResult_Ok : RecluseResult_Failed;
        m_maxQueryCount = maxQueries;
        m_type = type;

        if (result == RecluseResult_Ok)
        {
            D3D12_RESOURCE_DESC desc = { };
            desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            desc.Format = DXGI_FORMAT_UNKNOWN;
            desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            desc.MipLevels = 1;
            desc.Alignment = 0;
            desc.DepthOrArraySize = 1;
            desc.Height = 1;
            desc.Width = maxQueries * 16;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            D3D12_HEAP_PROPERTIES heapProps = { };
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.CreationNodeMask = 0;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.Type = D3D12_HEAP_TYPE_READBACK;
            heapProps.VisibleNodeMask = 0;

            hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, __uuidof(ID3D12Resource), (void**)&m_scratchBuffer);
            R_ASSERT(SUCCEEDED(hr));
        }
    }
    return result;
}


ResultCode D3D12QueryManager::release()
{
    if (m_heap)
        m_heap->Release();
    if (m_scratchBuffer)
        m_scratchBuffer->Release();
    m_heap          = nullptr;
    m_scratchBuffer = nullptr;
    return RecluseResult_Ok;
}


ResultCode D3D12QueryManager::reset()
{
    // Resets back to 0.
    m_currentAvailableIndex = 0;
    return RecluseResult_Ok;
}


D3D12QueryManager::Index D3D12QueryManager::beginQuery(ID3D12GraphicsCommandList* commandlist)
{
    U32 index = allocateIndex();
    if (index != GraphicsQuery::InvalidQuery)
    {
        switch (getQueryType())
        {
            case D3D12_QUERY_TYPE_TIMESTAMP: 
                // TODO(): Both vulkan and d3d12 have similar behavior with timestamp queries, we should make a special function case for them.
                commandlist->EndQuery(m_heap, getQueryType(), index);
                break;
            case D3D12_QUERY_TYPE_OCCLUSION:
            default:    
                commandlist->BeginQuery(m_heap, getQueryType(), index);
                break;
        }
    }
    return index;
}


D3D12QueryManager::Index D3D12QueryManager::allocateIndex()
{
    U32 index = GraphicsQuery::InvalidQuery;
    if ((m_currentAvailableIndex + 1) < m_maxQueryCount)
    {
        index = m_currentAvailableIndex;
        m_currentAvailableIndex += 1;
    }
    return index;
}


void D3D12QueryManager::endQuery(ID3D12GraphicsCommandList* list, Index query)
{
    switch (getQueryType())
    {
        case D3D12_QUERY_TYPE_TIMESTAMP: 
        {
            Index stopIndex = allocateIndex();
            R_ASSERT_FORMAT((stopIndex - 1) == query, "Query timestamps should be next to each other!");
            list->EndQuery(m_heap, getQueryType(), stopIndex);
            break;
        }
        case D3D12_QUERY_TYPE_OCCLUSION:
        default:
        {
            list->EndQuery(m_heap, getQueryType(), query);
            break;
        }
    }
}


void D3D12QueryManager::resolve(ID3D12GraphicsCommandList* commandlist)
{
    // No queries, skip.
    if (m_currentAvailableIndex == 0)
        return;

    D3D12_RESOURCE_BARRIER barrier = { };
    barrier.Transition.pResource = m_scratchBuffer;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.Subresource = 0;
    commandlist->ResourceBarrier(1, &barrier);

    commandlist->ResolveQueryData(m_heap, getQueryType(), 0, m_currentAvailableIndex, m_scratchBuffer, 0);

    barrier.Transition.pResource = m_scratchBuffer;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter =  D3D12_RESOURCE_STATE_COMMON;
    barrier.Transition.Subresource = 0;
    commandlist->ResourceBarrier(1, &barrier);
}


D3D12_QUERY_TYPE D3D12QueryManager::getQueryType() const
{
    switch (m_type)
    {
        case D3D12_QUERY_HEAP_TYPE_OCCLUSION:   return D3D12_QUERY_TYPE_OCCLUSION;
        case D3D12_QUERY_HEAP_TYPE_TIMESTAMP:   return D3D12_QUERY_TYPE_TIMESTAMP;
        default:                                return D3D12_QUERY_TYPE_TIMESTAMP;
    }
}
} // D3D12
} // Recluse