//
#include "Recluse/Messaging.hpp"
#include "Recluse/System/Window.hpp"

#include "Win32/Win32Common.hpp"
#include "Win32/Win32Runtime.hpp"
#include "Win32/IO/Win32Window.hpp"

#include <hidusage.h>

namespace Recluse {


static struct 
{
    B32 initialized;
} win32WindowFunctionality = { false };

CriticalSection windowCs = { };


static void setRawInputDevices(HWND hwnd)
{
    RAWINPUTDEVICE rid  = { };
    rid.hwndTarget      = hwnd;
    
    rid.usUsage     = HID_USAGE_GENERIC_MOUSE;
    rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid.dwFlags     = 0;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Failed to register raw input device to this window handle.");
    }
}


static B32 checkWindowRegistered()
{
    if (!win32WindowFunctionality.initialized) 
    {
        WNDCLASSEXW winClass = { };
        winClass.cbSize         = sizeof(WNDCLASSEXW);
        winClass.lpszClassName  = R_WIN32_WINDOW_NAME;
        winClass.hInstance      = GetModuleHandle(NULL);
        winClass.hIcon          = LoadIcon(GetModuleHandle(NULL), IDI_APPLICATION);
        winClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
        winClass.style          = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
        winClass.lpfnWndProc    = win32RuntimeProc;
        
        if (!RegisterClassExW(&winClass)) 
        {
            R_ERROR(R_CHANNEL_WIN32, "Failed to initialize native Win32 window system!");
            
            return win32WindowFunctionality.initialized;
        }
    
        win32WindowFunctionality.initialized = true;
        windowCs.initialize();
    }

    return win32WindowFunctionality.initialized;
}


R_INTERNAL
DWORD getWindowStyle(ScreenMode screenMode)
{
    switch (screenMode)
    {
        case ScreenMode_WindowBorderless:
        case ScreenMode_FullscreenBorderless:
            return (WS_POPUP);
        case ScreenMode_Windowed:
        default:
            return (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER | WS_SIZEBOX);
    }
}


Window* Window::create(const std::string& title, U32 x, U32 y, U32 width, U32 height, ScreenMode screenMode)
{
    R_DEBUG
        (
            R_CHANNEL_WIN32, 
            "Creating window: %s, X: %d Y: %d Width: %d, Height: %d", 
            title.c_str(), 
            x, y, 
            width, height
        );

    Window* pWindow     = nullptr; 
    HWND hwnd           = nullptr;

    if (!checkWindowRegistered()) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Failed to create window handle.");
        
        return nullptr;
    }

    {   
        wchar_t* ltitle = nullptr;
        int size        = 0;

        size = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), (int )title.size(), NULL, 0);

        ltitle = new wchar_t[size + 1];
        ltitle[size] = L'\0';

        MultiByteToWideChar(CP_UTF8, 0, title.c_str(), (int )title.size(), ltitle, size);

        hwnd = CreateWindowExW
                    (
                        NULL,
                        R_WIN32_WINDOW_NAME,
                        ltitle, 
                        (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER | WS_SIZEBOX), 
                        x, y, 
                        width, height, 
                        NULL, 
                        NULL, 
                        GetModuleHandle(NULL), NULL
                    );

        delete[] ltitle;
    }

    if (!hwnd) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Failed to create window handle!");
        
        return nullptr;
    }

    pWindow                 = new Window();
    pWindow->m_xPos         = x;
    pWindow->m_yPos         = y;
    pWindow->m_width        = width;
    pWindow->m_height       = height;
    pWindow->m_title        = title;
    pWindow->m_handle       = hwnd;
    pWindow->m_isBorderless = (screenMode == ScreenMode_FullscreenBorderless || screenMode == ScreenMode_WindowBorderless);
    pWindow->m_isFullscreen = (screenMode == ScreenMode_Fullscreen || screenMode == ScreenMode_FullscreenBorderless);
    pWindow->m_isMinimized  = false;
    pWindow->m_isShowing    = false;

    SetPropW(hwnd, R_WIN32_PROP_NAME, pWindow);

    pWindow->setScreenMode(screenMode);
    pWindow->update();

    setRawInputDevices(hwnd);

    UpdateWindow(hwnd);

    R_DEBUG(R_CHANNEL_WIN32, "Successfully created window!");

    return pWindow;
}


void Window::close()
{
    if (m_shouldClose) 
    {
        R_TRACE(R_CHANNEL_WIN32, "This window is already closed! Ignoring call...");
        return;
    }

    R_DEBUG(R_CHANNEL_WIN32, "Closing window...");    

    HWND hwnd = (HWND)getNativeHandle();

    BOOL result = DestroyWindow(hwnd);

    if (result == FALSE) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Failed to close window handle! Error: %d", GetLastError());
        return;
    }

    // Delete the API window.
    m_shouldClose = true;
    
    R_DEBUG(R_CHANNEL_WIN32, "Window successfully closed.");
}


ResultCode Window::destroy(Window* pWindow)
{
    R_DEBUG(R_CHANNEL_WIN32, "Destroying window...");

    BOOL result = FALSE;

    if (!pWindow) 
    {
        R_WARN(R_CHANNEL_WIN32, "Null window handle passed... Can not close. Ignoring...");

        return RecluseResult_NullPtrExcept;
    }

    if (!pWindow->shouldClose()) 
    {
        pWindow->close();
    }

    // delete the API window handle.
    delete pWindow;

    R_DEBUG(R_CHANNEL_WIN32, "Successfully destroyed window!");

    return RecluseResult_Ok;
}


void Window::setToCenter()
{
    HMONITOR monitor = MonitorFromWindow((HWND)m_handle, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO);

    GetMonitorInfo(monitor, &monitorInfo);
    RECT windowSize;
    GetWindowRect((HWND)m_handle, &windowSize);

    LONG monitorX = monitorInfo.rcMonitor.right;
    LONG monitorY = monitorInfo.rcMonitor.bottom;

    LONG centerX = (monitorX - windowSize.right) / 2;
    LONG centerY = (monitorY - windowSize.bottom) / 2;

    SetWindowPos((HWND)m_handle, 0, static_cast<int>(centerX), static_cast<int>(centerY), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    m_xPos = static_cast<int>(centerX);
    m_yPos = static_cast<int>(centerY);
    UpdateWindow((HWND)m_handle);
}


void Window::open()
{
    HWND hwnd = (HWND)getNativeHandle();
    ShowWindow(hwnd, SW_SHOW);
    //SetForegroundWindow(hwnd);
    UpdateWindow(hwnd);
    m_isShowing = true;
}


void Window::update()
{
    if (mustChangeScreen())
    {
        HWND hwnd = (HWND)m_handle;
        if (isBorderless())
        {
            SetWindowLongW(hwnd, GWL_EXSTYLE, 0);
            SetWindowLongW(hwnd, GWL_STYLE, (WS_POPUP));
            SetWindowPos(hwnd, HWND_NOTOPMOST, m_xPos, m_yPos, m_width, m_height, (SWP_FRAMECHANGED));
        }
        else
        {
            // Adjust window size due to possible menu.
            RECT windowRect = { (LONG)m_xPos, (LONG)m_yPos, (LONG)m_width, (LONG)m_height };

            AdjustWindowRect(&windowRect, WS_CAPTION, GetMenu(hwnd) != NULL);

            MoveWindow
                (
                    hwnd, 
                    0, 0,
                    windowRect.right - windowRect.left, 
                    windowRect.bottom - windowRect.top, 
                    FALSE
                );
        }
        screenChanged();
    }

    if (mustResize())
    {
        onWindowResize(m_xPos, m_yPos, m_width, m_height);
        resized();
    }
}


void Window::setScreenSize(U32 width, U32 height)
{
    m_width = width;
    m_height = height;
    m_status.mustResize = true;
    update();
}
} // Recluse