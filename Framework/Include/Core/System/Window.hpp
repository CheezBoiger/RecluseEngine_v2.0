// 
#pragma once

#include "Core/Types.hpp"

namespace Recluse {


enum ScreenMode {
    SCREEN_MODE_WINDOWED,
    SCREEN_MODE_FULLSCREEN,
    SCREEN_MODE_WINDOW_BORDERLESS
};

typedef void(*WindowMouseFunction)();
typedef void(*WindowKeyFunction)();

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
    static R_EXPORT ErrType destroy(Window* pWindow);

    // close the window.
    R_EXPORT void close();

    R_EXPORT void open();

    // obtain the native window handle.
    void* getNativeHandle() { return m_handle; }

    void setScreenMode(ScreenMode mode);

    void setTitle(const std::string& title);

    void setScreenSize(U32 width, U32 height);

    void setToCenter();

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
    U32 m_width;
    U32 m_height;
    U32 m_xPos;
    U32 m_yPos;
    ScreenMode m_screenMode;
    B32 m_isMinimized : 1;
    B32 m_shouldClose : 1;
    B32 m_isShowing   : 1;
    
    std::string m_title;
    void* m_handle;
};
} // Recluse