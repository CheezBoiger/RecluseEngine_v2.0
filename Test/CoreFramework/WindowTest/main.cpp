
#include <iostream>

#include "Recluse/System/KeyboardInput.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Time.hpp"

using namespace Recluse;

int main()
{
    Log::initializeLoggingSystem();

    // Initialize the realtime tick.
    RealtimeTick::initializeWatch(1ull, 0);
    Window* window = Window::create("WindowTest", 0, 0, 800, 600, ScreenMode_Windowed);
    
    R_ASSERT(window);

    window->setToCenter();
    window->show();

    // initialize our max time.
    F32 seconds     = 0.f;
    F32 maxTimeS    = 60.0f;

    while(!window->shouldClose()) 
    {
        RealtimeTick::updateWatch(1ull, 0);

        // Get the current tick.
        RealtimeTick tick   = RealtimeTick::getTick(0);
        seconds            += tick.delta();

        //R_VERBOSE("TIMING", "Current Time: %f, Delta Time: %f Seconds: %f", tick.getCurrentTimeS(), tick.delta(), seconds);
        R_VERBOSE("Window", "Window x: %d, y: %d, width: %d, height: %d", window->getPosX(), window->getPosY(), window->getWidth(), window->getHeight());

        KeyboardListener listener;
        
        if (listener.isKeyDown(KeyCode_F))
        {
            if (window->getScreenMode() != ScreenMode_Fullscreen)
                window->setScreenMode(ScreenMode_Fullscreen);
            else
            {
                window->setScreenSize(800, 600);
                window->setScreenMode(ScreenMode_Windowed);
            }
            window->update();
        }

        if (listener.isKeyDown(KeyCode_Escape))
            window->close();

        pollEvents();
    }

    Window::destroy(window);
    Log::destroyLoggingSystem();

    return 0;
}