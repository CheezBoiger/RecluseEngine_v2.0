
#include <iostream>

#include "Recluse/Time.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Renderer/RenderCommand.hpp"

#include <vector>
#include <queue>

using namespace Recluse;
using namespace Recluse::Engine;


IndexedInstancedSubMesh indexedSubmeshes[8];
InstancedSubMesh submeshes[12];

void fillList(RenderCommandList& list)
{
    ErrType result = R_RESULT_OK;
    DrawRenderCommand rcmd = { };
    DrawIndexedRenderCommand icmd = { };

    for (U32 i = 0; i < 8; ++i) {
        indexedSubmeshes[i].firstIndex = 0;
        indexedSubmeshes[i].firstInstance = 1;
        indexedSubmeshes[i].instanceCount = 1;
        indexedSubmeshes[i].pMaterial = nullptr;
        indexedSubmeshes[i].indexCount = 150;
    }

    for (U32 i = 0; i < 12; ++i) {
        submeshes[i].firstInstance = 1;
        submeshes[i].instanceCount = 1;
        submeshes[i].vertexCount = 12345;
    
    }

    rcmd.op = C_OP_DRAWABLE_INSTANCED;
    rcmd.numVertexBuffers = 2;
    rcmd.numSubMeshes = 12;
    rcmd.pSubMeshes = submeshes;

    icmd.op = C_OP_DRAWABLE_INDEXED_INSTANCED;
    icmd.numVertexBuffers = 1;
    icmd.indexType = INDEX_TYPE_UINT32;
    icmd.numSubMeshes = 8;
    icmd.pSubMeshes = indexedSubmeshes;

    for (U32 i = 0; i < 2000; ++i) {
        result = list.push(rcmd);
    }

    for (U32 i = 0; i < 6000; ++i) {
        result = list.push(icmd);
    }

    if (result != R_RESULT_OK) {
        R_ERR("TEST", "Failed to push some commands!");
    }
}

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);

    RenderCommandList list;

    list.initialize();

    RealtimeTick::updateWatch(1ull, 0);
    RealtimeTick tick = RealtimeTick::getTick(0);

    fillList(list);

    RealtimeTick::updateWatch(1ull, 0);
    tick = RealtimeTick::getTick(0);

    R_TRACE("TEST", "Took %f secs to fill commandlist.", tick.delta());

    RenderCommand** commands = list.getRenderCommands();
    U64 numRenderCommands = list.getNumberCommands();

    CommandOp op = C_OP_DRAWABLE_INDEXED_INSTANCED;

    for (U64 i = 0; i < numRenderCommands; ++i) {

        RenderCommand* cmd = commands[i];
        switch (cmd->op) {
    
            case C_OP_DRAWABLE_INSTANCED:
            {
                //R_TRACE("TEST", "I am a draw command! %d", i);
                DrawRenderCommand* rcmd = static_cast<DrawRenderCommand*>(cmd);
                op = rcmd->op;
                //R_TRACE("TEST", "%d", rcmd)
                break;
            }
            case C_OP_DRAWABLE_INDEXED_INSTANCED:    
            {
                //R_TRACE("TEST", "I am a draw indexed command! %d", i);
                DrawIndexedRenderCommand* icmd = static_cast<DrawIndexedRenderCommand*>(cmd);
                //R_TRACE("TEST", "%d", icmd);
                op = icmd->op;
                break;
            }
        }
    
    }

    RealtimeTick::updateWatch(1ull, 0);
    tick = RealtimeTick::getTick(0);
    R_VERBOSE("TEST", "Took %f secs to read list.", tick.delta());

    list.reset();
    list.destroy();
    Log::destroyLoggingSystem();
    return 0;
}