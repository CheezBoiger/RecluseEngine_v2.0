// 
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


enum ScreenMode {
    SCREEN_MODE_WINDOWED,
    SCREEN_MODE_FULLSCREEN,
    SCREEN_MODE_WINDOW_BORDERLESS
};

typedef void(*WindowMouseFunction)();
typedef void(*WindowKeyFunction)();
typedef void(*OnWindowResizeFunction)();
typedef void(*OnWindowRelocateFunction)();

// Window objects, to be instantiated and destroyed by the application.
// Rendering systems will need this in order to draw to the screen.
// This is an operating system dependent implementation.
class Window {
public:
    Window()
        : m_shouldClose(false)
        , m_handle(NULL)
        , m_height(0)
        , m_width(0)
        , m_xPos(0)
        , m_yPos(0)
        , m_isMinimized(false)
        , m_isShowing(false) { }

    // Create the window.
    static R_EXPORT Window* create(const std::string& title, U32 x, U32 y, U32 width, U32 height);

    // Destroy the window.
    static R_EXPORT ErrType destroy(Window* pWindow);

    // close the window.
    R_EXPORT void close();

    // Open the window, j
    R_EXPORT void open();

    // obtain the native window handle.
    void* getNativeHandle() { return m_handle; }

    // Set the screen mode.
    void setScreenMode(ScreenMode mode);

    // Set the title of the window.
    void setTitle(const std::string& title);

    // Programmatically resize the window.
    void setScreenSize(U32 width, U32 height);

    // Set the window to the center of your screen.
    void setToCenter();

    // Set the window to a location on your screen.
    void setToPosition(U32 x, U32 y);

    void restore();

    void minimize();

    void maximize();

    B32 isMinimized() const { return m_isMinimized; }

    U32 getWidth() const { return m_width; }
    
    U32 getHeight() const { return m_height; }

    U32 getPosX() const { return m_xPos; }

    U32 getPosY() const { return m_yPos; }

    const std::string& getTitle() const { return m_title; }

    ScreenMode getScreenMode() const { return m_screenMode; }

    B32 shouldClose() const { return m_shouldClose; }

private:
    // The window width
    U32 m_width;
    // The window height.
    U32 m_height;
    // The window x position (top-left corner.)
    U32 m_xPos;
    // The window y position (top-left corner.)
    U32 m_yPos;
    // Current screen mode.
    ScreenMode m_screenMode;
    // If the window is minimized.
    B32 m_isMinimized : 1;
    // Should the window close.
    B32 m_shouldClose : 1;
    // Is the window showing.
    B32 m_isShowing   : 1;
    // Title of the window.
    std::string m_title;
    void* m_handle;

    WindowKeyFunction       m_keyCallback;
    WindowMouseFunction     m_mouseCallback;
    OnWindowResizeFunction  m_onWindowResizeCallback;
    OnWindowRelocateFunction  m_windowRelocateCallback;
};
} // Recluse