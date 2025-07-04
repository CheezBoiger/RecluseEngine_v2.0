
#include <iostream>

#include "Recluse/System/Input.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Application.hpp"
#include "Recluse/System/KeyboardInput.hpp"

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Renderer/RenderCommand.hpp"
#include "Recluse/Renderer/Material.hpp"
#include "Recluse/Core/Profile/Profiler.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/Generated/RendererResources.hpp"

#include <vector>
#include <queue>

using namespace Recluse;
using namespace Recluse::Engine;


class TestApplication : public Application
{
public:

    virtual ResultCode onUpdate(TaskManager& manager) override
    {
        DrawBatch rcmd = {};
        //pRenderer->pushRenderCommand(rcmd, RENDER_PREZ);

        //R_VERBOSE("GameLoop", "time=%f fps", 1.f / tick.delta());
        //R_VERBOSE("GameLoop", "renderTime=%f fps", RealtimeTick::getTick(0).delta());
        
        pollEvents();

        if (m_window->shouldClose())
        {
            MessageBus::fireEvent(getMessageBus(), RenderEvent_Pause);
            stop();
        }
        
        RendererModule::getMain()->simLock();

        manager.pushTask(1, [] () -> ResultCode 
            {
                KeyboardListener listener;
                if (listener.isKeyDownOnce(KeyCode_0))
                {
                    GlobalCommands::setValue("Renderer.ClearFrame", GlobalCommands::obtainValue<Bool>("Renderer.ClearFrame") ? false : true);
                }

                if (listener.isKeyDownOnce(KeyCode_1))
                {
                    GlobalCommands::setValue("Renderer.UseClearColor", GlobalCommands::obtainValue<Bool>("Renderer.UseClearColor") ? false : true);
                }

                return RecluseResult_Ok;
            });

        RendererModule::getMain()->simUnlock();

        //CpuPerformanceProfile::PerformanceMeasurement measure = CpuPerformanceProfile::query("RandomTask", "Main");
        //R_NOTIFY("Main", "RandomTask: %f ms", measure.milliseconds);

        return RecluseResult_Ok;
    }

    virtual ResultCode onInit() override
    {
        RendererModule::initializeModule(this);

        m_window = Window::create("", 0, 0, 1200, 800);
        m_window->setToCenter();
        m_window->show();

        LogSystem::enableLogTypes(LogType_Debug | LogType_Notify);
        LogSystem::setLogChannel("Renderer", true);
        LogSystem::setLogChannel("MainProcess", true);
        LogSystem::setLogChannel("Application", true);

        RendererConfigs config = { };
        config.api = GraphicsApi_Direct3D12;
        config.enableGpuValidation = true;
        config.buffering = 3;
        config.maxFrameRate = 60.0f;
        config.windowHandle = m_window->getNativeHandle();
        config.renderWidth = m_window->getWidth();
        config.renderHeight = m_window->getHeight();
        RendererModule::getMain()->setNewConfigurations(config);
        RendererModule::getMain()->linkMessageBus(getMessageBus());

        MessageBus::fireEvent(getMessageBus(), RenderEvent_Initialize);
        MessageBus::fireEvent(getMessageBus(), RenderEvent_Resume);
        GlobalCommands::setValue("Renderer.ClearColor", Math::Color4(255, 0, 0, 0));
        // Make task process for the renderer.
        m_renderProcessId = makeTaskProcess(RendererModule::kRendererProcessTask, "Renderer");
#if 1
        makeTaskProcess([] (TaskProcess* process) -> ResultCode 
            {
                R_SCOPED_CPU_PROFILER(RandomTask, Math::Color4(1, 1, 1, 1), Main);
                std::array<U32, 5000> arr0;
                std::array<U32, 5000> arr1;
                std::array<U32, 5000> result;
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
                    //R_NOTIFY(process->getProcessName().c_str(), "%s", s.c_str());
                    return RecluseResult_Ok;
                });
                process->dispatchTasks();
                return RecluseResult_Ok;
            }, "RandomTask");
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

    ProcessId m_renderProcessId;
    Window* m_window;
};

int main(int c, char* argv[])
{
    LogSystem::initializeLoggingSystem();
    TestApplication testApp = {};
    MainThreadLoop::initialize();
    MainThreadLoop::loadApp(&testApp);
    MainThreadLoop::run();
    MainThreadLoop::cleanUp();
    LogSystem::destroyLoggingSystem();     
    return 0;
}