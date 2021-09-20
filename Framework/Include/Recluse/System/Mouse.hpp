//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/System/InputController.hpp"
#include "Recluse/Serialization/SerialTypes.hpp"

namespace Recluse {

// Max number of buttons supported by this engine.
#define MAX_MOUSE_BUTTONS 12

class Mouse : public IInputController {
public:
    Mouse()
        : m_xLastPosition(0.f)
        , m_yLastPosition(0.f)
        , m_xPosition(0.f)
        , m_yPosition(0.f)
        , m_isShowing(false)
        , m_isClamped(false)
        , m_isEnabled(false) { }

    virtual R_EXPORT ErrType setInput(const IInputFeedback& feedback) override;

    virtual R_EXPORT ErrType initialize(const std::string& controllerName) override;

    virtual R_EXPORT ErrType destroy() override;

    F32 getXPos() const { return m_xPosition; }
    F32 getYPos() const { return m_yPosition; }

    F32 getLastXPos() const { return m_xLastPosition; }
    F32 getLastYPos() const { return m_yLastPosition; }

    Bool getIsShowing() const { return m_isShowing; }
    Bool getIsClamped() const { return m_isClamped; }

    Bool getIsEnabled() const { return m_isEnabled; }

    void setEnable(Bool enable) { m_isEnabled = enable; }
    void setShowing(Bool show) { m_isShowing = show; }
    void setClamped(Bool clamped) { m_isClamped = clamped; }

    // Updates the position of this mouse, which will also 
    // update the last known position.
    void updatePosition(F32 x, F32 y) {
        m_xLastPosition = m_xPosition; 
        m_yLastPosition = m_yPosition; 
        m_xPosition = x; 
        m_yPosition = y; 
    }

private:
    F32         m_xPosition;
    F32         m_yPosition;
    F32         m_xLastPosition;
    F32         m_yLastPosition;
    InputState  m_buttonStates[MAX_MOUSE_BUTTONS];
    Bool        m_isShowing;
    Bool        m_isEnabled;
    Bool        m_isClamped;
};
} // Recluse