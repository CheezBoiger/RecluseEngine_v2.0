//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


enum InputState {
    INPUT_STATE_NONE,
    INPUT_STATE_DOWN,
    INPUT_STATE_UP
};


struct IInputFeedback {
    I32 xRate;
    I32 yRate;
    InputState state;
};

class IInputController {
public:
    virtual ~IInputController() { }
    
    virtual ErrType initialize(const std::string& controllerName) = 0;
    virtual ErrType destroy() = 0;

    virtual ErrType getInput(IInputFeedback& feedback) = 0;

    virtual ErrType integrateInput(const IInputFeedback& feedback) = 0;
};
} // Recluse