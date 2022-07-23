//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

// InputState is the state of our input.
// It is intended to be a bit flip type, so that it is quicker to process.
enum InputState 
{
    INPUT_STATE_UP,     // Must always be 0.
    INPUT_STATE_DOWN    // Must always be 1.
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
    
    virtual ErrType initialize(const std::string& controllerName) = 0;
    virtual ErrType destroy() = 0;

    virtual ErrType getInput(IInputFeedback& feedback) = 0;

    virtual ErrType integrateInput(const IInputFeedback& feedback) = 0;

    virtual void setButtonState(U32 buttonIx, InputState) = 0;
    virtual InputState getButtonState(U32 buttonIx) = 0;
};
} // Recluse