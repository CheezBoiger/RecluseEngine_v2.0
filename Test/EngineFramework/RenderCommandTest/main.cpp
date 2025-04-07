
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

void fillList(CommandList& list)
{
    ResultCode result = RecluseResult_Ok;
    RenderCommand cmd = { };
    DrawInstancedBatch rcmd = { };
    DrawIndexedBatch icmd = { };

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

    rcmd.numSubMeshes = 12;
    rcmd.pSubMeshes = submeshes;
    rcmd.topology = PrimitiveTopology_TriangleList;

    icmd.indexType = IndexType_Unsigned32;
    icmd.numSubMeshes = 8;
    icmd.pSubMeshes = indexedSubmeshes;
    icmd.topology = PrimitiveTopology_TriangleList;


    cmd.op = CommandOp_DrawInstanced;
    cmd.opData = &rcmd;
    for (U32 i = 0; i < 12000; ++i) {
        result = list.push(cmd);
    }

    cmd.op = CommandOp_DrawIndexedInstanced;
    cmd.opData = &icmd;
    for (U32 i = 0; i < 6000; ++i) {
        result = list.push(cmd);
    }

    if (result != RecluseResult_Ok) {
        R_ERROR("TEST", "Failed to push some commands!");
    }
}

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    enableLogTypes(LogType_Verbose);
    RealtimeTick::initializeWatch(1ull, 0);

    CommandList list;

    list.initialize(R_MB(16));

    RealtimeTick::updateWatch(1ull, 0);
    RealtimeTick tick = RealtimeTick::getTick(0);

    fillList(list);

    RealtimeTick::updateWatch(1ull, 0);
    tick = RealtimeTick::getTick(0);

    R_VERBOSE("TEST", "Took %f secs to fill commandlist.", tick.delta());

    RenderCommand* commands = list.getRenderCommands();
    U64 numRenderCommands = list.getNumberCommands();

    U32 totalSubmeshes = 0;
    U32 totalVertices = 0;

    for (U64 i = 0; i < numRenderCommands; ++i) {

        RenderCommand& cmd = commands[i];
        switch (cmd.op) 
        {
            case CommandOp_DrawInstanced:
            {
                //R_TRACE("TEST", "I am a draw command! %d", i);
                DrawInstancedBatch* rcmd = static_cast<DrawInstancedBatch*>(cmd.opData);
                //R_TRACE("TEST", "Draw call numSubmeshes=%d", rcmd->numSubMeshes);
                totalSubmeshes += rcmd->numSubMeshes;
                for (U32 i = 0; i < rcmd->numSubMeshes; ++i)
                {
                    InstancedSubMesh& submesh = rcmd->pSubMeshes[i];
                    totalVertices += submesh.vertexCount;
                } 
                break;
            }
            case CommandOp_DrawIndexedInstanced:    
            {
                //R_TRACE("TEST", "I am a draw indexed command! %d", i);
                DrawIndexedBatch* icmd = static_cast<DrawIndexedBatch*>(cmd.opData);
                totalSubmeshes += icmd->numSubMeshes;
                for (U32 i = 0; i < icmd->numSubMeshes; ++i)
                {
                    IndexedInstancedSubMesh& submesh = icmd->pSubMeshes[i];
                    totalVertices += submesh.indexCount;
                }
                //R_TRACE("TEST", "Indexed Draw call numSubmeshes=%d", icmd->numSubMeshes);
                break;
            }
        }
    }

    RealtimeTick::updateWatch(1ull, 0);
    tick = RealtimeTick::getTick(0);
    R_VERBOSE("TEST", "Total submeshes=%d, totalIndices=%d, totalVertices=%d, Took %f ms to read list.", totalSubmeshes, totalVertices, totalVertices, tick.delta() * 1000);

    list.reset();
    list.destroy();
    Log::destroyLoggingSystem();
    return 0;
}