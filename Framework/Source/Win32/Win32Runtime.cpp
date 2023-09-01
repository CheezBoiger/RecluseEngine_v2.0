//
#include "Win32/Win32Common.hpp"
#include "Win32/Win32Runtime.hpp"
#include "Win32/IO/Win32Window.hpp"
#include "Win32//IO/Win32Keyboard.hpp"

#include "Logging/LogFramework.hpp"

#include "Recluse/Time.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Mouse.hpp"

// Number of watch types available to the engine. This can vary, so be sure to update the cost needed.
#define MAX_WATCH_TYPE_INDICES      (16)
#define STOPWATCH_INDEX             (MAX_WATCH_TYPE_INDICES - 1)

namespace Recluse {


static U64 initializeTicksPerSecond() 
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}


static struct 
{
    // NOTE(): We don't really need a hash table or a map for this, 
    // since we are only storing a few keys/watches. Simple arrays will do.
    Win32RuntimeTick    ticks[MAX_WATCH_TYPE_INDICES]   = { };
    U64                 watchId[MAX_WATCH_TYPE_INDICES] = { };

    U64                 gTicksPerSecond = initializeTicksPerSecond();   //< ticks in seconds.
    RAWINPUT            lpb[4];                                            //< raw input.
    const DWORD         mainThreadId    = GetCurrentThreadId();         //< this is the main thread id!
    Bool                isInitialized   = false;
} gWin32Runtime;


// Call this first time on initialization, to initialize all our watch slots.
static void initializeWatchSlots()
{
    for (U32 i = 0; i < MAX_WATCH_TYPE_INDICES; ++i)
    {
        gWin32Runtime.watchId[i]    = 0;
        gWin32Runtime.ticks[i]      = { };
    }
}


#if defined(RECLUSE_DEBUG) || defined(RECLUSE_DEVELOPER)
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

    switch (hresult) 
    {
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
#endif

void enableOSColorInput()
{
    HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (stdOutHandle == INVALID_HANDLE_VALUE) 
    {        
        R_ERROR(R_CHANNEL_WIN32, "Unable to obtain standard output handle.");

        return;
    }

    DWORD dwMode = 0; 

    if (!GetConsoleMode(stdOutHandle, &dwMode)) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Unable to get output handle mode!");

        return;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;   
    
    if (!SetConsoleMode(stdOutHandle, dwMode)) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Unable to set the output handle mode for virtual terminal processing!");
    }
}


void Win32RuntimeTick::updateLastTimeS(U64 newLastTimeS, F32 delta)
{
    m_time          = newLastTimeS;
    m_currentTimeS  = F32(newLastTimeS);
    m_deltaTimeS    = delta;
}


U64 getTicksPerSecondS()
{
    return gWin32Runtime.gTicksPerSecond;
}


U64 Win32RuntimeTick::getLastTimeS() const
{
    return m_time;
}


U64 getCurrentTickS()
{
    LARGE_INTEGER newTick;
    QueryPerformanceCounter(&newTick);
    return newTick.QuadPart;
}


void RealtimeTick::updateWatch(U64 id, U32 watchType)
{
    R_ASSERT(watchType < MAX_WATCH_TYPE_INDICES);

    if (gWin32Runtime.watchId[watchType] == 0)
    {
        R_ERROR
            (
                "RealtimeTick", 
                "This watch=%d is not initialized! Can not update!", 
                watchType
            );
        return;
    }

    if (gWin32Runtime.watchId[watchType] != id)
    {
        R_ERROR
            (
                "RealtimeTick", 
                "Can not update watch=%d. Id=%llu does not own it!", 
                watchType, id
            );
        return;
    }

    // Otherwise, lets just update.

    Win32RuntimeTick& nativeTick    = gWin32Runtime.ticks[watchType];
    
    const U64 ticksPerSecond        = getTicksPerSecondS();
    const U64 lastTimeS             = nativeTick.getLastTimeS();
    const U64 currentTimeS          = getCurrentTickS();

    F32 fDeltaTime                  = F32(currentTimeS - lastTimeS) / F32(ticksPerSecond);

    nativeTick.updateLastTimeS(currentTimeS, fDeltaTime);
}


RealtimeTick::RealtimeTick(U32 watchType)
{
    R_ASSERT(watchType < MAX_WATCH_TYPE_INDICES);

    if (gWin32Runtime.watchId[watchType] == 0)
    {
        R_WARN("RealtimeTick", "Can't query uninitialized watch=%d! Likely not initialized yet.", watchType);
        return;
    }
    
    const Win32RuntimeTick& nativeTick = gWin32Runtime.ticks[watchType];
    
    m_currentTimeS  = nativeTick.getCurrentTime();
    m_deltaTimeS    = nativeTick.getDelta();
}


void RealtimeTick::initializeWatch(U64 id, U32 watchType)
{
    R_ASSERT(watchType < MAX_WATCH_TYPE_INDICES);

    // Check if we need to initialize any global params.
    if (!gWin32Runtime.isInitialized)
    {
        initializeTicksPerSecond();
        initializeWatchSlots();
        gWin32Runtime.isInitialized             = true;
    }

    // If the watch is already initialized with the id, then we ignore init.
    // Otherwise, proceed with the intialization.
    if 
        (
            gWin32Runtime.watchId[watchType] != 0 &&
            gWin32Runtime.watchId[watchType] != id
        )
    {
        R_ERROR("RealtimeTick", "Watch type is already initialized! Ignoring...");
        return;
    }
   
    gWin32Runtime.watchId[watchType]    = id;
    gWin32Runtime.ticks[watchType]      = Win32RuntimeTick();
}


RealtimeTick RealtimeTick::getTick(U32 watchType)
{
    return RealtimeTick(watchType);
}

#define CHECK_KEY_STATE_DOWN(keyCode, registerFn) { \
    SHORT s = GetKeyState(keyCode); \
    if (s & 0x8000) registerFn(I32(keyCode), WM_KEYDOWN); \
  }

#define CHECK_KEY_STATE_UP(keyCode, registerFn) { \
    SHORT s = GetKeyState(keyCode); \
    if (~(s & 0x8000)) registerFn(I32(keyCode), WM_KEYUP); \
  }


#pragma optimize("", off)
LRESULT CALLBACK win32RuntimeProc(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Reinterpret hwnd to window pointer.
    Window* pWindow = reinterpret_cast<Window*>(GetPropW(hwnd, R_WIN32_PROP_NAME));

    if (pWindow)
    {
        switch (uMsg) 
        {
            case WM_CLOSE:
            case WM_QUIT:
            {
                pWindow->close();
                break;
            }
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_INPUT:
            {
                RAWINPUT* raw   = gWin32Runtime.lpb;
                UINT dwSize;
                UINT result     = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &dwSize, sizeof(RAWINPUTHEADER)); 
                Mouse* pMouse   = pWindow->getMouseHandle();
                if (result != (UINT)-1) 
                {                
                    if (pMouse) 
                    {
                        IInputFeedback feedback = { };
                        I32 dx = 0, dy = 0;
                        if (raw->header.dwType == RIM_TYPEMOUSE) 
                        {
                            if (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) 
                            {
                                I32 prevX = pMouse->getLastXPos();
                                I32 prevY = pMouse->getLastYPos();

                                dx = raw->data.mouse.lLastX  - prevX; // get the delta from last mouse pos.
                                dy = raw->data.mouse.lLastY  - prevY; // get the delta from last mouse pos.
                            } 
                            else 
                            {
                                dx = raw->data.mouse.lLastX;
                                dy = raw->data.mouse.lLastY;
                            }

                            feedback.buttonStateFlags = pMouse->allButtonFlags();
                            for (U32 i = 1, index = 0; index < 5; i <<= 2, ++index)
                            {
                                if (raw->data.mouse.ulButtons & i)
                                    feedback.buttonStateFlags |= (1 << index);
                                else if (raw->data.mouse.ulButtons & (i << 1))
                                    feedback.buttonStateFlags &= ~(1 << index);
                            }
                        } 

                        feedback.xRate = dx;
                        feedback.yRate = dy;

                        // TODO: Set the mouse position.
                        pMouse->integrateInput(feedback);
                    }
                }
                else // I am not sure why we need this, but the compiler keeps optimizing the code above out, without it.
                    R_WARN("Raw Input", "Raw Input returned incorrect results");
                break;
            }
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                Win32::registerKeyCall(I32(wParam), WM_KEYDOWN);
                // Shift/ctrl key is registered, but we also need to check which shift/ctrl key (left, right)
                // was also pressed. We check both. 
                if (wParam == VK_SHIFT)
                {
                    CHECK_KEY_STATE_DOWN(VK_LSHIFT, Win32::registerKeyCall);
                    CHECK_KEY_STATE_DOWN(VK_RSHIFT, Win32::registerKeyCall);
                }
                else if (wParam == VK_CONTROL)
                {
                    CHECK_KEY_STATE_DOWN(VK_LCONTROL, Win32::registerKeyCall);
                    CHECK_KEY_STATE_DOWN(VK_RCONTROL, Win32::registerKeyCall);
                }
                else if (wParam == VK_MENU)
                {
                    CHECK_KEY_STATE_DOWN(VK_LMENU, Win32::registerKeyCall);
                    CHECK_KEY_STATE_DOWN(VK_RMENU, Win32::registerKeyCall);
                }
                break;
            }
            case WM_SYSKEYUP:
            case WM_KEYUP:
            {
                Win32::registerKeyCall(I32(wParam), WM_KEYUP);
                // The same applies as above key_down call.
                if (wParam == VK_SHIFT)
                {
                    CHECK_KEY_STATE_UP(VK_LSHIFT, Win32::registerKeyCall);
                    CHECK_KEY_STATE_UP(VK_RSHIFT, Win32::registerKeyCall);
                }
                else if (wParam == VK_CONTROL)
                {
                    CHECK_KEY_STATE_UP(VK_LCONTROL, Win32::registerKeyCall);
                    CHECK_KEY_STATE_UP(VK_RCONTROL, Win32::registerKeyCall);
                }
                else if (wParam == VK_MENU)
                {
                    CHECK_KEY_STATE_UP(VK_LMENU, Win32::registerKeyCall);
                    CHECK_KEY_STATE_UP(VK_RMENU, Win32::registerKeyCall);
                }
                break;
            }
            case WM_MOVE:
            {
                UINT x = LOWORD(lParam);
                UINT y = HIWORD(lParam);
                pWindow->overridePosition(x, y);
                break;
            }
            case WM_SIZE:
            {
                // As a window is resized, we relay this back to the handler.
                // Keep in mind that this needs to be relay'ed back to the renderer. 
                // Which shouldn't be problematic. We can signal this inside our onWindowResize callback.
                UINT width  = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                pWindow->setScreenSize(width, height);
                switch (wParam)
                {
                    case SIZE_MINIMIZED:
                    {
                        pWindow->overrideMinimized(true);
                        break;
                    }
                    case SIZE_RESTORED:
                    {
                        pWindow->overrideMinimized(false);
                        break;
                    }
                    case SIZE_MAXIMIZED:
                    {
                        pWindow->overrideMinimized(false);
                        break;
                    }
                }
                break;
            }
            //case WM_MOUSEMOVE:
            //case WM_SYSCOMMAND:
            //case WM_SHOWWINDOW:
            //{
            //    break;
            //}
            //case WM_PAINT:
            default: break;
        }    
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}


void pollEvents()
{
    MSG msg;
    // Poll win32 information (keyboard, mouse, etc...)
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Poll controller information if needed.
    DWORD dwResult;
    for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        XINPUT_STATE state;

        ZeroMemory(&state, sizeof(XINPUT_STATE));

        dwResult = XInputGetState(i, &state);

        if (dwResult == ERROR_SUCCESS)
        {
            // Connected to a controller.
            R_WARN(R_CHANNEL_WIN32, "We are connected to a controller!");
        }
    }
}
#pragma optimize("", on)

U64 getMainThreadId()
{
    return (U64)gWin32Runtime.mainThreadId;
}


U64 getCurrentThreadId()
{
    return (U64)GetCurrentThreadId();
}


RealtimeStopWatch::RealtimeStopWatch()
{
    if (!gWin32Runtime.isInitialized)
    {
        initializeTicksPerSecond();
        gWin32Runtime.isInitialized = true;
    }

    const U64 currentTime = getCurrentTickS();

    m_currentTimeU64 = currentTime;
}


RealtimeStopWatch::operator Recluse::RealtimeTick()
{
    RealtimeTick tick = RealtimeTick();
    tick.m_currentTimeS = F32(m_currentTimeU64);
    tick.m_deltaTimeS = F32(m_currentTimeU64) / F32(getTicksPerSecondS());
    return tick;
}


RealtimeStopWatch RealtimeStopWatch::operator-(const RealtimeStopWatch& rh)
{
    RealtimeStopWatch watch;
    watch.m_currentTimeU64 = m_currentTimeU64 - rh.m_currentTimeU64;
    return watch;
}
} // Recluse