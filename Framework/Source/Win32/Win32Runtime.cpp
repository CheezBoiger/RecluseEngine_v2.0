//
#include "Win32/Win32Common.hpp"
#include "Win32/Win32Runtime.hpp"
#include "Win32/IO/Win32Window.hpp"

#include "Logging/LogFramework.hpp"

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Mouse.hpp"

namespace Recluse {

static struct {
    U64 gTicksPerSecond;
    U64 gTime;
    RAWINPUT lpb;
} gWin32Runtime;

namespace Asserts {

Result AssertHandler::check(Bool cond, const char* functionStr, const char* msg)
{
    if (cond) return ASSERT_OK;

    std::string message = "";
    message += functionStr;
    message += "\n\n";
    message += msg;

    // If the assert is false, we should handle it.
    DWORD hresult = MessageBox(NULL, msg, NULL, MB_ABORTRETRYIGNORE);

    switch (hresult) {
        case IDRETRY:
            return ASSERT_DEBUG;
        case IDABORT:
            return ASSERT_TERMINATE;
        case IDCANCEL:
            return ASSERT_IGNORE;
        default:
            break;
    }

    return ASSERT_IGNORE;
}
} // Asserts


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
            IInputFeedback feedback = { };
            Mouse* pMouse           = pWindow->getMouseHandle();
            RAWINPUT* raw           = &gWin32Runtime.lpb;

            I32 dx = 0, dy = 0;
            UINT dwSize;

            if (!pMouse) {
                // Break early as there is no mouse attached to this 
                // window.
                break;
            }
        
            if (GetRawInputData((HRAWINPUT)lParam, 
                                RID_INPUT, 
                                raw, 
                                &dwSize, 
                                sizeof(RAWINPUTHEADER)) == -1u) 
            {

                R_WARN(R_CHANNEL_WIN32, "Raw input not returning correct size.");
                break;

            }

            if (raw->header.dwType == RIM_TYPEMOUSE) {
                if (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
                    I32 prevX = pMouse->getLastXPos();
                    I32 prevY = pMouse->getLastYPos();

                    dx = raw->data.mouse.lLastX - prevX; // get the delta from last mouse pos.
                    dy = raw->data.mouse.lLastY - prevY; // get the delta from last mouse pos.
        
                } else {
                    dx = raw->data.mouse.lLastX;
                    dy = raw->data.mouse.lLastY;
                }
            }

            feedback.state = INPUT_STATE_NONE;
            feedback.xRate = dx;
            feedback.yRate = dy;

            // TODO: Set the mouse position.
            pMouse->integrateInput(feedback);

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
        case WM_PAINT:
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