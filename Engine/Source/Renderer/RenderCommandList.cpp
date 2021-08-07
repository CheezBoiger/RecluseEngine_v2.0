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


void RenderCommandList::initialize()
{
    // 2 MB for our render command list.
    U64 szBytes = R_ALLOC_MASK(2 * R_1MB, ARCH_PTR_SZ_BYTES);
    if (!m_pool) {

        m_pool = new MemoryPool(szBytes);
        m_pAllocator = new StackAllocator();

        m_pAllocator->initialize(m_pool->getBaseAddress(), m_pool->getTotalSizeBytes());

    }

    if (!m_pointerPool) {
        szBytes = R_ALLOC_MASK(64 * R_1KB, ARCH_PTR_SZ_BYTES);
        m_pointerPool = new MemoryPool(szBytes);
        m_pointerAllocator = new StackAllocator();
    
        m_pointerAllocator->initialize(m_pointerPool->getBaseAddress(), m_pointerPool->getTotalSizeBytes());
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

    if (m_pointerPool) {
    
        m_pointerAllocator->cleanUp();
        delete m_pointerPool;
        delete m_pointerAllocator;

        m_pointerPool = nullptr;
        m_pointerAllocator = nullptr;    

    }
}

#define COPY_COMMAND_TO_POOL(rClass, rcmd) { \
    result = m_pAllocator->allocate(&allocation, sizeof(rClass), ARCH_PTR_SZ_BYTES); \
    if (result == REC_RESULT_OUT_OF_MEMORY) { \
        R_ERR("RenderCommandList", "Command list is out of memory! Can not push render command!"); \
        return result; \
    } \
    *((rClass*)allocation.ptr) = static_cast<const rClass&>(rcmd); \
}

ErrType RenderCommandList::push(const RenderCommand& renderCommand)
{
    Allocation allocation = { };
    ErrType result = REC_RESULT_OK;

    switch (renderCommand.op) {

        case COMMAND_OP_DRAW_INSTANCED:
        {
            COPY_COMMAND_TO_POOL(DrawRenderCommand, renderCommand);  
            break;     
        }
        case COMMAND_OP_DRAW_INDEXED_INSTANCED:
        {
            COPY_COMMAND_TO_POOL(DrawIndexedRenderCommand, renderCommand); 
            break;
        }
    }

    if (result == REC_RESULT_OK) {
        PtrType p = allocation.ptr;
        result = m_pointerAllocator->allocate(&allocation, ARCH_PTR_SZ_BYTES, ARCH_PTR_SZ_BYTES);

        if (result == REC_RESULT_OK) {
            *(RenderCommand**)allocation.ptr = (RenderCommand*)p;
        }
    }
    
    return result;
}


void RenderCommandList::reset()
{
    m_pAllocator->reset();
    m_pointerAllocator->reset();
}
} // Engine
} // Recluse