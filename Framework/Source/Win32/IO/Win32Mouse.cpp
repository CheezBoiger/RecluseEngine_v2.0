//
#include "Recluse/System/Mouse.hpp"

namespace Recluse {

ErrType Mouse::integrateInput(const IInputFeedback& feedback) 
{
    
    I32 xPosition = m_xPosition + feedback.xRate;
    I32 yPosition = m_yPosition + feedback.yRate;

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