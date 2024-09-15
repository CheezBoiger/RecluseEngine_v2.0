
#include <iostream>

#include "Recluse/System/Input.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Application.hpp"

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Renderer/RenderCommand.hpp"
#include "Recluse/Renderer/Material.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/Generated/RendererResources.hpp"

#include <vector>
#include <queue>

using namespace Recluse;
using namespace Recluse::Engine;


class TestApplication : public Application
{
public:

    virtual ResultCode onUpdate() override
    {
        DrawRenderCommand rcmd = {};
        rcmd.op = CommandOp_DrawableInstanced;
        rcmd.vertexTypeFlags = VERTEX_ATTRIB_POSITION | VERTEX_ATTRIB_NORMAL;
        rcmd.numSubMeshes = 0;
        //pRenderer->pushRenderCommand(rcmd, RENDER_PREZ);

        //R_VERBOSE("GameLoop", "time=%f fps", 1.f / tick.delta());
        //R_VERBOSE("GameLoop", "renderTime=%f fps", RealtimeTick::getTick(0).delta());

        pollEvents();

        if (m_window->shouldClose())
        {
            stop();
        }
        return RecluseResult_Ok;
    }

    virtual ResultCode onInit() override
    {
        Renderer::initializeModule(this);

        m_window = Window::create("", 0, 0, 1200, 800);
        m_window->setToCenter();
        m_window->show();

        enableLogTypes(LogType_Debug | LogType_Notify);
        setLogChannel("Renderer", true);
        setLogChannel("MainProcess", true);
        setLogChannel("Application", true);

        RendererConfigs config = { };
        config.api = GraphicsApi_Direct3D12;
        config.enableGpuValidation = true;
        config.buffering = 3;
        config.maxFrameRate = 60.0f;
        config.windowHandle = m_window->getNativeHandle();
        config.renderWidth = m_window->getWidth();
        config.renderHeight = m_window->getHeight();
        Renderer::getMain()->setNewConfigurations(config);

        MessageBus::fireEvent(getMessageBus(), RenderEvent_Initialize);
        MessageBus::fireEvent(getMessageBus(), RenderEvent_Resume);

        R_NOTIFY("Renderer", "RUIN");
        // Make task process for the renderer.
        makeTaskProcess(Renderer::kRendererProcessTask);
#if 1
        makeTaskProcess([] (TaskProcess* process) -> ResultCode 
        {
            std::array<U32, 10> arr0;
            std::array<U32, 10> arr1;
            std::array<U32, 10> result;
            process->pushTask(0, [&] () -> ResultCode 
            {
                for (U32 i = 0; i < arr0.size(); ++i)
                    arr0[i] = i * 2;
                return RecluseResult_Ok; 
            });
            process->pushTask(0, [&] () -> ResultCode 
            {
                for (U32 i = 0; i < arr1.size(); ++i)
                {
                    arr1[i] = i+1;
                }
                return RecluseResult_Ok;
            });
            // Push the final task.
            process->pushTask(1, [&] () -> ResultCode 
            {
                std::string s = "";
                for (U32 i = 0; i < result.size(); ++i)
                {
                    result[i] = arr0[i] + arr1[1];
                    s += " " + std::to_string(result[i]);
                }
                R_NOTIFY("MainProcess", "%s", s.c_str());
                return RecluseResult_Ok;
            });
            process->dispatchTasks();
            return RecluseResult_Ok;
        });
#endif
        return RecluseResult_Ok;
    }
    virtual ResultCode onCleanUp() override
    {
        MessageBus::fireEvent(getMessageBus(), RenderEvent_Shutdown);

        getMessageBus()->notifyAll();

        Window::destroy(m_window);
        return RecluseResult_Ok;
    }

    Window* m_window;
};

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    TestApplication testApp = {};
    MainThreadLoop::initialize();
    MainThreadLoop::loadApp(&testApp);
    MainThreadLoop::run();
    MainThreadLoop::cleanUp();

    Engine::Material material("PBR_RoughMetal");

    material.declare("RoughMetal")
        .declare("Albedo")
        .declare("Normal")
        .declare("config");
    Log::destroyLoggingSystem();     
    return 0;
}