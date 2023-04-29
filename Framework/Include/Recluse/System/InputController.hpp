//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

// InputState is the state of our input.
// It is intended to be a bit flip type, so that it is quicker to process.
enum InputState 
{
    InputState_Up,     // Must always be 0.
    InputState_Down    // Must always be 1.
};


struct IInputFeedback 
{
    I32 xRate;
    I32 yRate;
    U32 buttonStateFlags;
    U32 button;
};

class IInputController 
{
public:
    virtual ~IInputController() { }
    
    virtual ResultCode initialize(const std::string& controllerName) = 0;
    virtual ResultCode destroy() = 0;

    virtual ResultCode getInput(IInputFeedback& feedback) = 0;

    virtual ResultCode integrateInput(const IInputFeedback& feedback) = 0;

    virtual void setButtonState(U32 buttonIx, InputState) = 0;
    virtual InputState getButtonState(U32 buttonIx) = 0;
};
} // Recluse