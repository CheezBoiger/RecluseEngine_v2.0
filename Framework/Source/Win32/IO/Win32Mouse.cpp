//
#include "Recluse/System/Mouse.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {

ResultCode Mouse::integrateInput(const IInputFeedback& feedback) 
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

    return RecluseResult_Ok;
}


ResultCode Mouse::initialize(const std::string& controllerName)
{
    return RecluseResult_NoImpl;
}


ResultCode Mouse::destroy()
{
    return RecluseResult_NoImpl;
}


ResultCode Mouse::getInput(IInputFeedback& feedback)
{
    return RecluseResult_NoImpl;
}


ResultCode Mouse::setIconPath(const std::string& iconPath)
{
    return RecluseResult_NoImpl;
}
} // Recluse