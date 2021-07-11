//
#include "Recluse/Messaging.hpp"
#include "Recluse/System/Window.hpp"

#include "Win32/Win32Common.hpp"
#include "Win32/Win32Runtime.hpp"
#include "Win32/IO/Win32Window.hpp"

namespace Recluse {


static struct {
    B32 initialized;
} win32WindowFunctionality = { false };


B32 checkWindowRegistered()
{
    if (!win32WindowFunctionality.initialized) {
        WNDCLASSEXW winClass = { };
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.lpszClassName = R_WIN32_WINDOW_NAME;
        winClass.hInstance = GetModuleHandle(NULL);
        winClass.hIcon = LoadIcon(GetModuleHandle(NULL), IDI_APPLICATION);
        winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        winClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
        winClass.lpfnWndProc = win32RuntimeProc;
        
        if (!RegisterClassExW(&winClass)) {
    
            R_ERR(R_CHANNEL_WIN32, "Failed to initialize native Win32 window system!");
            
            return win32WindowFunctionality.initialized;
        }
    
        win32WindowFunctionality.initialized = true;
    }

    return win32WindowFunctionality.initialized;
}


Window* Window::create(const std::string& title, U32 x, U32 y, U32 width, U32 height)
{
    R_DEBUG(R_CHANNEL_WIN32, "Creating window: %s, X: %d Y: %d Width: %d, Height: %d", 
        title.c_str(), x, y, width, height);

    Window* pWindow     = nullptr; 
    HWND hwnd           = nullptr;

    if (!checkWindowRegistered()) {

        R_ERR(R_CHANNEL_WIN32, "Failed to create window handle.");
        
        return nullptr;

    }

    {    
        wchar_t* ltitle = nullptr;
        int size        = 0;

        size = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), (int )title.size(), NULL, 0);

        ltitle = new wchar_t[size + 1];
        ltitle[size] = L'\0';

        MultiByteToWideChar(CP_UTF8, 0, title.c_str(), (int )title.size(), ltitle, size);

        hwnd = CreateWindowExW(NULL, R_WIN32_WINDOW_NAME,
            ltitle, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), 
            x, y, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);

        delete[] ltitle;
    }

    if (!hwnd) {
    
        R_ERR(R_CHANNEL_WIN32, "Failed to creat widnow handle!");
        
        return nullptr;

    }

    pWindow             = new Window();
    pWindow->m_xPos     = x;
    pWindow->m_yPos     = y;
    pWindow->m_width    = width;
    pWindow->m_height   = height;
    pWindow->m_title    = title;
    pWindow->m_handle   = hwnd;

    SetPropW(hwnd, R_WIN32_PROP_NAME, pWindow);

    // Adjust window size due to possible menu.
    RECT windowRect = { (LONG)x, (LONG)y, (LONG)width, (LONG)height };

    AdjustWindowRect(&windowRect, WS_CAPTION, GetMenu(hwnd) != NULL);

    MoveWindow(hwnd, 0, 0,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, FALSE);

    UpdateWindow(hwnd);

    R_DEBUG(R_CHANNEL_WIN32, "Successfully created window!");

    return pWindow;
}


void Window::close()
{
    if (m_shouldClose) {

        R_TRACE(R_CHANNEL_WIN32, "This window is already closed! Ignoring call...");
        return;

    }

    R_DEBUG(R_CHANNEL_WIN32, "Closing window...");    

    HWND hwnd = (HWND)getNativeHandle();

    BOOL result = DestroyWindow(hwnd);

    if (result == FALSE) {

        R_ERR(R_CHANNEL_WIN32, "Failed to close window handle! Error: %d", GetLastError());

        return;
    }

    // Delete the API window.
    m_shouldClose = true;
    
    R_DEBUG(R_CHANNEL_WIN32, "Window successfully closed.");
}


ErrType Window::destroy(Window* pWindow)
{
    R_DEBUG(R_CHANNEL_WIN32, "Destroying window...");

    BOOL result = FALSE;

    if (!pWindow) {

        R_WARN(R_CHANNEL_WIN32, "Null window handle passed... Can not close. Ignoring...");

        return REC_RESULT_NULL_PTR_EXCEPTION;
    }

    if (!pWindow->shouldClose()) {

        pWindow->close();

    }

    // delete the API window handle.
    delete pWindow;

    R_DEBUG(R_CHANNEL_WIN32, "Successfully destroyed window!");

    return REC_RESULT_OK;
}


void Window::open()
{
    HWND hwnd = (HWND)getNativeHandle();
    ShowWindow(hwnd, SW_SHOW);
    m_isShowing = true;
}
} // Recluse