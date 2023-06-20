// Copyright (c) Recluse Engine, 2021.
#include "Recluse/Renderer/RenderCommand.hpp"

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Engine {


void RenderCommandList::initialize()
{
    // 2 MB for our render command list.
    U64 szBytes = align(2 * R_1MB, pointerSizeBytes());
    if (!m_pool) 
    {
        m_pool = new MemoryPool(szBytes);
        m_pAllocator = new LinearAllocator();

        m_pAllocator->initialize(m_pool->getBaseAddress(), m_pool->getTotalSizeBytes());
    }

    if (!m_pointerPool) 
    {
        szBytes = align(64 * R_1KB, pointerSizeBytes());
        m_pointerPool = new MemoryPool(szBytes);
        m_pointerAllocator = new LinearAllocator();
    
        m_pointerAllocator->initialize(m_pointerPool->getBaseAddress(), m_pointerPool->getTotalSizeBytes());
    }
}


void RenderCommandList::destroy()
{
    if (m_pool) 
    {
        m_pAllocator->cleanUp();

        delete m_pool;
        delete m_pAllocator;
    
        m_pool = nullptr;
        m_pAllocator = nullptr;
    }

    if (m_pointerPool) 
    {
        m_pointerAllocator->cleanUp();
        delete m_pointerPool;
        delete m_pointerAllocator;

        m_pointerPool = nullptr;
        m_pointerAllocator = nullptr;
    }
}

#define COPY_COMMAND_TO_POOL(rClass, rcmd) \
    { \
        result = m_pAllocator->allocate(&allocation, sizeof(rClass), pointerSize()); \
        if (result == RecluseResult_OutOfMemory) { \
            R_ERROR("RenderCommandList", "Command list is out of memory! Can not push render command!"); \
            return result; \
        } \
        *((rClass*)allocation.baseAddress) = static_cast<const rClass&>(rcmd); \
    }

ResultCode RenderCommandList::push(const RenderCommand& renderCommand)
{
    UPtr allocation = kNullPtr;

    switch (renderCommand.op) 
    {
        case CommandOp_DrawableInstanced:
        {
            //COPY_COMMAND_TO_POOL(DrawRenderCommand, renderCommand);
            DrawRenderCommand* pCommand = new (m_pAllocator) DrawRenderCommand();
            allocation = (UPtr)pCommand;
            *pCommand = static_cast<const DrawRenderCommand&>(renderCommand);
            break;     
        }

        case CommandOp_DrawableIndexedInstanced:
        {
            //COPY_COMMAND_TO_POOL(DrawIndexedRenderCommand, renderCommand); 
            DrawIndexedRenderCommand* pCommand = new (m_pAllocator) DrawIndexedRenderCommand();
            allocation = (UPtr)pCommand;
            *pCommand = static_cast<const DrawIndexedRenderCommand&>(renderCommand);
            break;
        }
    }

    UPtr ptrAlloc = m_pointerAllocator->allocate(pointerSizeBytes(), pointerSizeBytes());
    ResultCode result = m_pointerAllocator->getLastError();
    if (result == RecluseResult_Ok) 
    {
        *(RenderCommand**)ptrAlloc = (RenderCommand*)allocation;
    }
    
    return RecluseResult_Ok;
}


void RenderCommandList::reset()
{
    m_pAllocator->reset();
    m_pointerAllocator->reset();
}
} // Engine
} // Recluse