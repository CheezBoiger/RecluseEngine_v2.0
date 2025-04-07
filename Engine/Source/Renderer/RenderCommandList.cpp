// Copyright (c) Recluse Engine, 2021.
#include "Recluse/Renderer/RenderCommand.hpp"

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Memory/LinearScratchMemory.hpp"

namespace Recluse {
namespace Engine {


CommandList::CommandList()
        : m_pAllocator(nullptr)
        , m_scratchAllocator(nullptr)
        , m_pool(nullptr)
        , m_scratch(nullptr) 
{ 
}

CommandList::~CommandList()
{
    destroy();
}

void CommandList::initialize(U32 scratchSizeBytes)
{
    // 2 MB for our render command list.
    U64 szBytes = align(R_MB(2) + sizeof(LinearAllocator), pointerSizeBytes());
    if (!m_pool) 
    {
        m_pool = new MemoryPool(szBytes);
        m_pAllocator = new ((void*)m_pool->getBaseAddress()) LinearAllocator();
        m_pAllocator->initialize(m_pool->getPtrAddressAt(sizeof(LinearAllocator)), m_pool->getTotalSizeBytes() - sizeof(LinearAllocator));
    }

    if (!m_scratch)
    {
        szBytes = align(scratchSizeBytes + sizeof(LinearAllocator), pointerSizeBytes());
        m_scratch = new MemoryPool(szBytes);
        m_scratchAllocator = new ((void*)m_scratch->getBaseAddress()) LinearAllocator();
        m_scratchAllocator->initialize(m_scratch->getPtrAddressAt(sizeof(LinearAllocator)), m_scratch->getTotalSizeBytes() - sizeof(LinearAllocator));
    }
}


void CommandList::destroy()
{
    if (m_pool) 
    {
        m_pAllocator->cleanUp();
        
        delete m_pool;
        m_pool = nullptr;
        m_pAllocator;
    }

    if (m_scratch)
    {
        m_scratchAllocator->cleanUp();
        delete m_scratch;
        m_scratch = nullptr;
        m_scratchAllocator = nullptr;
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

ResultCode CommandList::push(const RenderCommand& renderCommand)
{
    UPtr allocation = kNullPtr;

    switch (renderCommand.op) 
    {
        case CommandOp_DrawInstanced:
        {
            //COPY_COMMAND_TO_POOL(DrawRenderCommand, renderCommand);
            DrawInstancedBatch* cmd = CommandList::cast<DrawInstancedBatch>(renderCommand.opData);
            DrawInstancedBatch* pCommand = new (m_scratchAllocator) DrawInstancedBatch();
            R_ASSERT(m_scratchAllocator->getLastError() == RecluseResult_Ok);
            pCommand->numSubMeshes = cmd->numSubMeshes;
            InstancedSubMesh* submeshes = new (m_scratchAllocator) InstancedSubMesh[cmd->numSubMeshes];
            R_ASSERT(m_scratchAllocator->getLastError() == RecluseResult_Ok);
            
            for (U32 i = 0; i < cmd->numSubMeshes; ++i)
            {
                InstancedSubMesh& dstSubmesh = submeshes[i];
                InstancedSubMesh& srcSubmesh = cmd->pSubMeshes[i];
                dstSubmesh.firstInstance = srcSubmesh.firstInstance;
                dstSubmesh.firstVertex = srcSubmesh.firstVertex;
                dstSubmesh.instanceCount = srcSubmesh.instanceCount;
                dstSubmesh.vertexCount = srcSubmesh.vertexCount;
                dstSubmesh.pMaterial = srcSubmesh.pMaterial;
            }

            pCommand->pSubMeshes = submeshes;
            allocation = (UPtr)pCommand;
            break;     
        }

        case CommandOp_DrawIndexedInstanced:
        {
            //COPY_COMMAND_TO_POOL(DrawIndexedRenderCommand, renderCommand); 
            DrawIndexedBatch* cmd = CommandList::cast<DrawIndexedBatch>(renderCommand.opData);
            DrawIndexedBatch* pCommand = new (m_scratchAllocator) DrawIndexedBatch();

            R_ASSERT(m_scratchAllocator->getLastError() == RecluseResult_Ok);

            pCommand->indexType = cmd->indexType;
            pCommand->numSubMeshes = cmd->numSubMeshes;
            pCommand->offset = cmd->offset;
            pCommand->pIndexBuffer = cmd->pIndexBuffer;

            R_ASSERT(cmd->numSubMeshes > 0);
            IndexedInstancedSubMesh* submeshes = new (m_scratchAllocator) IndexedInstancedSubMesh[cmd->numSubMeshes];

            R_ASSERT(m_scratchAllocator->getLastError() == RecluseResult_Ok);

            for (U32 i = 0; i < cmd->numSubMeshes; ++i)
            {
                IndexedInstancedSubMesh& submesh = submeshes[i];
                IndexedInstancedSubMesh& srcSubmesh = cmd->pSubMeshes[i];

                submesh.firstIndex = srcSubmesh.firstIndex;
                submesh.firstInstance = srcSubmesh.firstInstance;
                submesh.indexCount = srcSubmesh.indexCount;
                submesh.instanceCount = srcSubmesh.instanceCount;
                submesh.vertexOffset = srcSubmesh.vertexOffset;
                submesh.pMaterial = srcSubmesh.pMaterial;
            }
            pCommand->pSubMeshes = submeshes;

            allocation = (UPtr)pCommand;
            break;
        }
    }

    UPtr ptrAlloc = m_pAllocator->allocate(sizeof(RenderCommand), 1ull);
    ResultCode result = m_pAllocator->getLastError();
    if (result == RecluseResult_Ok) 
    {
        RenderCommand* allocatedCmd = new ((void*)ptrAlloc) RenderCommand();
        allocatedCmd->op = renderCommand.op;
        allocatedCmd->stencilRef = renderCommand.stencilRef;
        allocatedCmd->opData = (void*)allocation;
    }
    
    return RecluseResult_Ok;
}


void CommandList::reset()
{
    m_pAllocator->reset();
    m_scratchAllocator->reset();
}


U64 CommandList::getNumberCommands() const 
{ 
    return m_pAllocator->getTotalAllocations(); 
}
} // Engine
} // Recluse