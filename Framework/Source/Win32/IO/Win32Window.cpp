//
#include "Recluse/Messaging.hpp"
#include "Recluse/System/Window.hpp"

#include "Win32/Win32Common.hpp"
#include "Win32/Win32Runtime.hpp"
#include "Win32/IO/Win32Window.hpp"

#include <hidusage.h>
#include <map>

namespace Recluse {


static struct 
{
    B32 initialized;
} win32WindowFunctionality = { false };

CriticalSection windowCs = { };

std::map<HWND, Window*> g_hwndToWindowMap;

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
        case ScreenMode_Fullscreen:
            return (WS_POPUP | WS_VISIBLE);
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

    g_hwndToWindowMap.insert(std::make_pair(hwnd, pWindow));

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

    g_hwndToWindowMap.erase((HWND)pWindow->getNativeHandle());
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


void Window::show()
{
    HWND hwnd = (HWND)getNativeHandle();
    ShowWindow(hwnd, SW_SHOW);
    //SetForegroundWindow(hwnd);
    UpdateWindow(hwnd);
    m_isShowing = true;
    // It is no longer minimized.
    m_isMinimized = false;
}


void Window::setScreenMode(ScreenMode mode)
{
    if (m_screenMode != mode)
    {
        HWND hwnd = (HWND)getNativeHandle();
        m_screenMode = mode;
        if (m_screenMode == ScreenMode_Windowed)
        {
            m_isBorderless = false;
            m_isFullscreen = false;
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
            UpdateWindow(hwnd);
        }
        
        if (m_screenMode == ScreenMode_WindowBorderless)
        {
            m_isBorderless = true;
            m_isFullscreen = false;
            SetWindowLongW(hwnd, GWL_EXSTYLE, 0);
            SetWindowLongW(hwnd, GWL_STYLE, (WS_POPUP));
            SetWindowPos(hwnd, HWND_NOTOPMOST, m_xPos, m_yPos, m_width, m_height, (SWP_FRAMECHANGED));
        }

        if (m_screenMode == ScreenMode_FullscreenBorderless)
        {
            m_isBorderless = true;
            m_isFullscreen = true;
        }

        if (m_screenMode == ScreenMode_Fullscreen)
        {
            m_isBorderless = false;
            m_isFullscreen = true;
            // Exclusive fullscreen
            DWORD dwStyle = GetWindowLongW(hwnd, GWL_STYLE);
            DWORD exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            MONITORINFO monitorInfo = { };
            monitorInfo.cbSize = sizeof(MONITORINFO);
            SetWindowLongW(hwnd, GWL_STYLE, dwStyle & ~(WS_CAPTION | WS_THICKFRAME) & (WS_POPUP | WS_VISIBLE));
            SetWindowLongW(hwnd, GWL_EXSTYLE, exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE) & (WS_EX_TOPMOST)); 
            GetMonitorInfoW(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &monitorInfo);
            const RECT rect = monitorInfo.rcMonitor;
            SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, (SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED));
            UpdateWindow(hwnd);
        }
    }
}


void Window::minimize()
{
    HWND hwnd = (HWND)m_handle;
    ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
    m_status.mustMinimize = true;
}


void Window::restore()
{
    HWND hwnd = (HWND)m_handle;
    ShowWindow(hwnd, SW_RESTORE);
    m_status.mustRestore = true;
}


void Window::update()
{
    HWND hwnd = (HWND)m_handle;
    if (mustResize())
    {
        resized();
        onWindowResize(m_xPos, m_yPos, m_width, m_height);
    }
    if (m_status.mustMinimize)
    {
        m_status.mustMinimize = false;
        m_isShowing = false; 
        m_isMinimized = true;
    }
    if (m_status.mustRestore)
    {
        m_isShowing = true;
        m_isMinimized = false;
        m_status.mustRestore = false;
        onWindowResize(m_xPos, m_yPos, m_width, m_height);
    }
}


void Window::setScreenSize(U32 width, U32 height)
{
    if (m_width != width || m_height != height)
    {
        m_width = width;
        m_height = height;
        m_status.mustResize = true;
    }
}


Window* getWindowAssociatedWithHwnd(HWND hwnd)
{
    auto& iter = g_hwndToWindowMap.find(hwnd);
    if (iter != g_hwndToWindowMap.end())
    {
        return iter->second;
    }
    return nullptr;
}
} // Recluse