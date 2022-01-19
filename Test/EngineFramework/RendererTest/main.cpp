
#include <iostream>

#include "Recluse/System/Input.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Application.hpp"

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Renderer/RenderCommand.hpp"

#include <vector>
#include <queue>

using namespace Recluse;
using namespace Recluse::Engine;


class TestApplication : public Application
{
public:

    virtual void update(const RealtimeTick& tick) override
    {
       DrawRenderCommand rcmd = {};
       rcmd.op = COMMAND_OP_DRAWABLE_INSTANCED;
       rcmd.vertexTypeFlags = VERTEX_ATTRIB_POSITION | VERTEX_ATTRIB_NORMAL;
       rcmd.numSubMeshes = 0;
       //pRenderer->pushRenderCommand(rcmd, RENDER_PREZ);

       R_VERBOSE("GameLoop", "time=%f fps", 1.f / tick.getDeltaTimeS());
    }

    virtual ErrType onInit() override
    {
        Log::initializeLoggingSystem();
        RealtimeTick::initializeWatch(0);
        Renderer::initializeModule(this);

        RenderMessage message = { };
        message.pApp = this;
        message.req = RenderMessage::RESUME;
        getMessageBus()->pushMessage(message);
        return REC_RESULT_OK;
    }

    virtual ErrType onCleanUp() override
    {
        RenderMessage message = { };
        message.pApp = this;
        message.req = RenderMessage::SHUTDOWN;
        getMessageBus()->pushMessage(message);
        Renderer::cleanUpModule(this);
        Log::destroyLoggingSystem();     
        return REC_RESULT_OK;
    }
};

int main(int c, char* argv[])
{
    TestApplication testApp = {};
    MainThreadLoop::initialize();
    MainThreadLoop::loadApp(&testApp);
    MainThreadLoop::run();
    MainThreadLoop::cleanUp();
    return 0;
}