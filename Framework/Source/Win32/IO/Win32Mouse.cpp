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

    return REC_RESULT_OK;
}


ErrType Mouse::initialize(const std::string& controllerName)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType Mouse::destroy()
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType Mouse::getInput(IInputFeedback& feedback)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType Mouse::setIconPath(const std::string& iconPath)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse