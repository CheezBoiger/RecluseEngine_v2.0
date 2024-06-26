
#include <iostream>

#include "Recluse/System/Input.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Application.hpp"

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Renderer/RenderCommand.hpp"

#include "Recluse/Generated/RendererResources.hpp"

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
        rcmd.op = CommandOp_DrawableInstanced;
        rcmd.vertexTypeFlags = VERTEX_ATTRIB_POSITION | VERTEX_ATTRIB_NORMAL;
        rcmd.numSubMeshes = 0;
        //pRenderer->pushRenderCommand(rcmd, RENDER_PREZ);

        //R_VERBOSE("GameLoop", "time=%f fps", 1.f / tick.delta());
        R_VERBOSE("GameLoop", "renderTime=%f fps", RealtimeTick::getTick(JobType_Renderer).delta());
    }

    virtual ResultCode onInit() override
    {
        Renderer::initializeModule(this);
        enableLogTypes(LogType_Debug);
        setLogChannel("Renderer", true);

        RendererConfigs config = { };
        config.api = GraphicsApi_Vulkan;
        config.enableGpuValidation = false;
        config.buffering = 3;
        config.maxFrameRate = 60.0f;
        config.windowHandle = getWindow()->getNativeHandle();
        config.renderWidth = getWindow()->getWidth();
        config.renderHeight = getWindow()->getHeight();
        Renderer::getMain()->setNewConfigurations(config);

        MessageBus::fireEvent(getMessageBus(), RenderEvent_Initialize);
        MessageBus::fireEvent(getMessageBus(), RenderEvent_Resume);
        return RecluseResult_Ok;
    }

    virtual ResultCode onCleanUp() override
    {
        MessageBus::fireEvent(getMessageBus(), RenderEvent_Shutdown);
        return RecluseResult_Ok;
    }
};

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    TestApplication testApp = {};
    MainThreadLoop::initialize();
    MainThreadLoop::loadApp(&testApp);
    MainThreadLoop::run();
    MainThreadLoop::cleanUp();        
    Log::destroyLoggingSystem();     
    return 0;
}