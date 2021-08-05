// Copyright (c) Recluse Engine, 2021.
#include "Recluse/Renderer/RenderCommand.hpp"

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/StackAllocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Engine {


void RenderCommandList::initialize(U32 numRenderCommands)
{
    U64 szBytes = R_ALLOC_MASK(numRenderCommands * sizeof(RenderCommand), ARCH_PTR_SZ_BYTES);
    if (m_pool) {

        m_pool = new MemoryPool(szBytes);
        m_pAllocator = new StackAllocator();

        m_pAllocator->initialize(m_pool->getBaseAddress(), m_pool->getTotalSizeBytes());

    }
}


void RenderCommandList::destroy()
{
    if (m_pool) {
    
        m_pAllocator->cleanUp();

        delete m_pool;
        delete m_pAllocator;
    
        m_pool = nullptr;
        m_pAllocator = nullptr;
    
    }
}


void RenderCommandList::push(const RenderCommand& renderCommand)
{
    Allocation allocation = { };
    ErrType result = REC_RESULT_OK;

    result = m_pAllocator->allocate(&allocation, sizeof(RenderCommand), ARCH_PTR_SZ_BYTES);

    if (result == REC_RESULT_OUT_OF_MEMORY) {
    
        // TODO: Handle memory issue.
        R_ERR("RenderCommandList", "Command list is out of memory! Can not push render command!");
        return;
    }

    // Copy render command to this region.
    *((RenderCommand*)allocation.ptr) = renderCommand;
    
}


void RenderCommandList::reset()
{
    m_pAllocator->reset();
}
} // Engine
} // Recluse