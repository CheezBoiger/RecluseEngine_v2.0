//
#pragma once

#include "Core/Types.hpp"

namespace Recluse {

typedef ErrType (*ThreadFunction)(void* data);

struct R_EXPORT Thread {
    ThreadFunction  func;
    U32             resultCode;
    void*           payload;
    SizeT           uid;
};


R_EXPORT ErrType createThread(Thread* thread, ThreadFunction startRoutine);
R_EXPORT ErrType detachThread(Thread* thread);
R_EXPORT ErrType joinThread(Thread* thread);
R_EXPORT ErrType killThread(Thread* thread);
} // Recluse