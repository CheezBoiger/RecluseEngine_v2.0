//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/System/InputController.hpp"
#include "Recluse/Serialization/SerialTypes.hpp"

namespace Recluse {

// Max number of buttons supported by this engine.
#define MAX_MOUSE_BUTTONS 12

class Mouse : public IInputController 
{
public:
    Mouse()
        : m_xLastPosition(0.f)
        , m_yLastPosition(0.f)
        , m_xPosition(0.f)
        , m_yPosition(0.f)
        , m_isShowing(false)
        , m_isClamped(false)
        , m_isEnabled(false) { }

    virtual R_PUBLIC_API ErrType    integrateInput(const IInputFeedback& feedback) override;
    virtual R_PUBLIC_API ErrType    initialize(const std::string& controllerName) override;
    virtual R_PUBLIC_API ErrType    destroy() override;
    virtual R_PUBLIC_API ErrType    getInput(IInputFeedback& feedback) override;
    R_PUBLIC_API ErrType            setIconPath(const std::string& iconPath);

    I32                             getXPos() const { return m_xPosition; }
    I32                             getYPos() const { return m_yPosition; }

    I32                             getLastXPos() const { return m_xLastPosition; }
    I32                             getLastYPos() const { return m_yLastPosition; }

    Bool                            getIsShowing() const { return m_isShowing; }
    Bool                            getIsClamped() const { return m_isClamped; }

    Bool                            getIsEnabled() const { return m_isEnabled; }

    void                            setEnable(Bool enable) { m_isEnabled = enable; }
    void                            setShowing(Bool show) { m_isShowing = show; }

    // Set the mouse to clamp to window.
    void                            setClamped(Bool clamped) { m_isClamped = clamped; }

    // Updates the position of this mouse, which will also 
    // update the last known position.
    void updatePosition(I32 x, I32 y) 
    {
        m_xLastPosition = m_xPosition; 
        m_yLastPosition = m_yPosition; 
        m_xPosition = x; 
        m_yPosition = y; 
    }

private:
    I32         m_xPosition;
    I32         m_yPosition;
    I32         m_xLastPosition;
    I32         m_yLastPosition;
    InputState  m_buttonStates[MAX_MOUSE_BUTTONS];
    Bool        m_isShowing;
    Bool        m_isEnabled;
    Bool        m_isClamped;
};
} // Recluse