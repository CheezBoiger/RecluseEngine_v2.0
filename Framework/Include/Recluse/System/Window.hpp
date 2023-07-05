// 
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


enum ScreenMode 
{
    ScreenMode_Windowed,
    ScreenMode_Fullscreen,
    ScreenMode_WindowBorderless,
    ScreenMode_FullscreenBorderless
};

class Mouse;

typedef void(*WindowMouseFunction)();
typedef void(*WindowKeyFunction)();
typedef void(*OnWindowResizeFunction)(U32, U32, U32, U32);
typedef void(*OnWindowRelocateFunction)();

// Monitor description information, usually handled when querying multiple monitors from 
// the operating system.
struct MonitorDesc
{
    U32 nativeWidth;
    U32 nativeHeight;
    F32 refreshRate;
    // Native handle to the monitor, should only read if necessary!
    void* handle;
    Bool isPrimary;
};


//////////////////////////////////////////////////////////////////////////////////
// Monitor querying api calls, this is usually specific to each operating system.
//
//
R_PUBLIC_API R_OS_CALL extern U32              getMonitorCount();
R_PUBLIC_API R_OS_CALL extern Bool             queryMonitors(MonitorDesc* pDescsOut, U32 count);
R_PUBLIC_API R_OS_CALL extern MonitorDesc      getActiveMonitor();
//
//////////////////////////////////////////////////////////////////////////////////

// Window objects, to be instantiated and destroyed by the application.
// Rendering systems will need this in order to draw to the screen.
// This is an operating system dependent implementation.
class R_OS_CALL Window 
{
public:
    Window()
        : m_shouldClose(false)
        , m_handle(NULL)
        , m_height(0)
        , m_width(0)
        , m_xPos(0)
        , m_yPos(0)
        , m_isMinimized(false)
        , m_monitorIndex(0)
        , m_isShowing(false)
        , m_isFullscreen(false)
        , m_isBorderless(false)
        , m_pMouseHandle(nullptr)
        , m_keyCallback(nullptr)
        , m_onWindowResizeCallback(nullptr) 
    { 
        m_status.mustChangeScreen = false; 
    }

    // Create the window.
    static R_OS_CALL R_PUBLIC_API Window*       create(const std::string& title, U32 x, U32 y, U32 width, U32 height, ScreenMode screenMode = ScreenMode_Windowed);
    // Destroy the window.
    static R_OS_CALL R_PUBLIC_API ResultCode    destroy(Window* pWindow);
    static R_OS_CALL R_PUBLIC_API Window*       getActiveFocusedWindow();

    // close the window.
    R_OS_CALL R_PUBLIC_API void close();
    // Open the window,
    R_OS_CALL R_PUBLIC_API void open();
    // obtain the native window handle.
    void*                       getNativeHandle() { return m_handle; }
    // Set the title of the window.
    void                        setTitle(const std::string& title);
    // Programmatically resize the window.
    void                        setScreenSize(U32 width, U32 height);
    // Set the window to the center of your screen.
    R_OS_CALL R_PUBLIC_API void setToCenter();
    // Set the window to a location on your screen.
    R_OS_CALL R_PUBLIC_API void setToPosition(U32 x, U32 y);
    void                        restore();
    void                        minimize();
    void                        maximize();
    B32                         isMinimized() const { return m_isMinimized; }
    B32                         isFullscreen() const { return m_isFullscreen; }
    B32                         isBorderless() const { return m_isBorderless; }
    U32                         getWidth() const { return m_width; }
    U32                         getHeight() const { return m_height; }
    U32                         getPosX() const { return m_xPos; }
    U32                         getPosY() const { return m_yPos; }
    const std::string&          getTitle() const { return m_title; }
    ScreenMode                  getScreenMode() const { return m_screenMode; }

    // Request a resize of this window.
    void                        requestResize(U32 newWidth, U32 newHeight);

    // Check if we should close the window.
    B32                         shouldClose() const { return m_shouldClose; }

    // set a mouse handle for this window. Only one mouse can be set per window.
    void                        setMouseHandle(Mouse* pMouse) { m_pMouseHandle = pMouse; }
    Mouse*                      getMouseHandle() { return m_pMouseHandle; }

    // Calls on window resize function. Should probbaly not be called outside of the Engine module.
    void                        onWindowResize(U32 x, U32 y, U32 width, U32 height) const { if (m_onWindowResizeCallback) m_onWindowResizeCallback(x, y, width, height); }

    // Get the monitor index that this window is focused on.
    U32                         getMonitorIndex() const { return m_monitorIndex; }

    // Grabs the display monitor that has the most area of intesection with the window.
    R_PUBLIC_API MonitorDesc    getCurrentActiveMonitor();

    // Set the screen mode.
    R_PUBLIC_API void setScreenMode(ScreenMode mode)
    {
        if (m_screenMode != mode)
        {
            m_screenMode = mode;
            m_status.mustChangeScreen = true;
        }
    }

private:

    B32 mustChangeScreen() const { return m_status.mustChangeScreen; }
    void screenChanged() { m_status.mustChangeScreen = false; }
    void update();

    // The window width
    U32                         m_width;
    // The window height.
    U32                         m_height;
    // The window x position (top-left corner.)
    U32                         m_xPos;
    // The window y position (top-left corner.)
    U32                         m_yPos;
    // Current screen mode.
    ScreenMode                  m_screenMode;
    // If the window is minimized.
    B32                         m_isMinimized : 1;
    // Should the window close.
    B32                         m_shouldClose : 1;
    // Is the window showing.
    B32                         m_isShowing   : 1;
    // Is the window in fullscreen?
    B32                         m_isFullscreen : 1;
    // Is the window borderless?
    B32                         m_isBorderless : 1;

    struct 
    {
        B32     mustChangeScreen;
    } m_status;

    // Title of the window.
    std::string                 m_title;
    void*                       m_handle;
    U32                         m_monitorIndex;

    WindowKeyFunction           m_keyCallback;
    WindowMouseFunction         m_mouseCallback;
    OnWindowResizeFunction      m_onWindowResizeCallback;
    OnWindowRelocateFunction    m_windowRelocateCallback;

    Mouse* m_pMouseHandle;
};
} // Recluse