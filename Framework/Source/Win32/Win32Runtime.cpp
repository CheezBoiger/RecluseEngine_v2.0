//
#include "Win32/Win32Common.hpp"
#include "Win32/Win32Runtime.hpp"
#include "Win32/IO/Win32Window.hpp"

#include "Logging/LogFramework.hpp"

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/System/Window.hpp"

namespace Recluse {


static struct {
    U64 gTicksPerSecond;
    U64 gTime;
    BYTE lpb[1 << 32];
} gWin32Runtime;


void enableOSColorInput()
{
    HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (stdOutHandle == INVALID_HANDLE_VALUE) {
            
        R_ERR(R_CHANNEL_WIN32, "Unable to obtain standard output handle.");

        return;

    }

    DWORD dwMode = 0; 

    if (!GetConsoleMode(stdOutHandle, &dwMode)) {

        R_ERR(R_CHANNEL_WIN32, "Unable to get output handle mode!");

        return;

    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;   
    
    if (!SetConsoleMode(stdOutHandle, dwMode)) {
    
        R_ERR(R_CHANNEL_WIN32, "Unable to set the output handle mode for virtual terminal processing!");
    
    }
}


static void initializePerformanceTimer()
{
    LARGE_INTEGER freq;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);

    gWin32Runtime.gTicksPerSecond   = freq.QuadPart;
    gWin32Runtime.gTime             = counter.QuadPart;
}


void updateLastTimeS(U64 newLastTimeS)
{
    gWin32Runtime.gTime = newLastTimeS;
}


U64 getTicksPerSecondS()
{
    return gWin32Runtime.gTicksPerSecond;
}


U64 getLastTimeS()
{
    return gWin32Runtime.gTime;
}


U64 getCurrentTickS()
{
    LARGE_INTEGER newTick;
    QueryPerformanceCounter(&newTick);
    return newTick.QuadPart;
}

RealtimeTick::RealtimeTick()
{
    U64 ticksPerSecond  = getTicksPerSecondS();
    U64 lastTimeS       = getLastTimeS();
    U64 currentTimeS    = getCurrentTickS();

    m_deltaTimeS        = F32(currentTimeS - lastTimeS) / ticksPerSecond;
    m_currentTimeS      = F32(currentTimeS);
    gWin32Runtime.gTime = currentTimeS;
}


void RealtimeTick::initialize()
{
    initializePerformanceTimer();
}


RealtimeTick RealtimeTick::getTick()
{
    return RealtimeTick();
}


LRESULT CALLBACK win32RuntimeProc(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Reinterpret hwnd to window pointer.
    Window* pWindow = reinterpret_cast<Window*>(GetPropW(hwnd, R_WIN32_PROP_NAME));

    switch (uMsg) {

        case WM_CLOSE:
        case WM_QUIT:
        {
            pWindow->close();
            break;
        }
        case WM_INPUT:
        {
            RAWINPUT* raw = (RAWINPUT*)gWin32Runtime.lpb;
            U32 dx, dy;
            UINT dwSize;
        
            if (GetRawInputData((HRAWINPUT)lParam, 
                                RID_INPUT, 
                                gWin32Runtime.lpb, 
                                &dwSize, 
                                sizeof(RAWINPUTHEADER)) == -1u) 
            {

                R_WARN(R_CHANNEL_WIN32, "Raw input not returning correct size.");
                break;

            }

            if (raw->header.dwType == RIM_TYPEMOUSE) {
                if (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
                    dx = raw->data.mouse.lLastX; // get the delta from last mouse pos.
                    dy = raw->data.mouse.lLastY; // get the delta from last mouse pos.
                } else {
                    dx = raw->data.mouse.lLastX;
                    dy = raw->data.mouse.lLastY;
                }
            }

            // TODO: Set the mouse position.

            break;
        }
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_MOVE:
        case WM_SIZE:
        case WM_MOUSEMOVE:
        case WM_SYSCOMMAND:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        default: break;
    }    

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}


void pollEvents()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    
    }
}
} // Recluse