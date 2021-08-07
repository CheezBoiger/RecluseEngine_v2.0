
#include <iostream>

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Renderer/RenderCommand.hpp"

#include <vector>
#include <queue>

using namespace Recluse;
using namespace Recluse::Engine;

void fillList(RenderCommandList& list)
{
    ErrType result = REC_RESULT_OK;
    DrawRenderCommand rcmd = { };
    DrawIndexedRenderCommand icmd = { };

    rcmd.firstInstance = 12;
    rcmd.op = COMMAND_OP_DRAW_INSTANCED;
    rcmd.numVertexBuffers = 2;
    rcmd.vertexCount = 10000;

    icmd.op = COMMAND_OP_DRAW_INDEXED_INSTANCED;
    icmd.numVertexBuffers = 1;
    icmd.firstIndex = 44;
    icmd.indexCount = 32;

    for (U32 i = 0; i < 2000; ++i) {
        result = list.push(rcmd);
    }

    for (U32 i = 0; i < 6000; ++i) {
        result = list.push(icmd);
    }

    if (result != REC_RESULT_OK) {
        R_ERR("TEST", "Failed to push some commands!");
    }
}

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initialize();

    RenderCommandList list;

    list.initialize();

    RealtimeTick tick = RealtimeTick::getTick();

    fillList(list);

    tick = RealtimeTick::getTick();

    R_TRACE("TEST", "Took %f secs to fill commandlist.", tick.getDeltaTimeS());

    RenderCommand** commands = list.getRenderCommands();
    U64 numRenderCommands = list.getNumberCommands();

    CommandOp op = COMMAND_OP_DRAW_INDEXED_INSTANCED;

    for (U64 i = 0; i < numRenderCommands; ++i) {

        RenderCommand* cmd = commands[i];
        switch (cmd->op) {
    
            case COMMAND_OP_DRAW_INSTANCED:
            {
                //R_TRACE("TEST", "I am a draw command! %d", i);
                DrawRenderCommand* rcmd = static_cast<DrawRenderCommand*>(cmd);
                op = rcmd->op;
                //R_TRACE("TEST", "%d", rcmd->op)
                break;
            }
            case COMMAND_OP_DRAW_INDEXED_INSTANCED:    
            {
                //R_TRACE("TEST", "I am a draw indexed command! %d", i);
                DrawIndexedRenderCommand* icmd = static_cast<DrawIndexedRenderCommand*>(cmd);
                //R_TRACE("TEST", "%d", icmd->firstIndex);
                op = icmd->op;
                break;
            }
        }
    
    }

    tick = RealtimeTick::getTick();
    R_VERBOSE("TEST", "Took %f secs to read list.", tick.getDeltaTimeS());

    list.reset();
    list.destroy();
    Log::destroyLoggingSystem();
    return 0;
}