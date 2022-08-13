//
#include "Recluse/System/Mouse.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {

ErrType Mouse::integrateInput(const IInputFeedback& feedback) 
{
    
    I32 xPosition = m_xPosition + feedback.xRate;
    I32 yPosition = m_yPosition + feedback.yRate;

    // We need to ensure that the number of buttons is less than the number of supported button flags.
    R_ASSERT(R_MAX_MOUSE_BUTTONS < 32);

    for (U32 i = 1, index = 0; index < R_MAX_MOUSE_BUTTONS; i <<= 1, ++index)
    {
        InputState requestedState   = (InputState)(feedback.buttonStateFlags & i);
        setButtonState(index, requestedState);
    }

    updatePosition(xPosition, yPosition);

    return R_RESULT_OK;
}


ErrType Mouse::initialize(const std::string& controllerName)
{
    return R_RESULT_NO_IMPL;
}


ErrType Mouse::destroy()
{
    return R_RESULT_NO_IMPL;
}


ErrType Mouse::getInput(IInputFeedback& feedback)
{
    return R_RESULT_NO_IMPL;
}


ErrType Mouse::setIconPath(const std::string& iconPath)
{
    return R_RESULT_NO_IMPL;
}
} // Recluse