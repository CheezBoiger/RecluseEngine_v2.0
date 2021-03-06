//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

typedef ErrType (*ThreadFunction)(void* data);

enum ThreadResultCode {
    THREAD_RESULT_UNKNOWN       = -999,
    THREAD_RESULT_NOT_READY     = -2,
    THREAD_RESULT_FAILED        = -1,
    THREAD_RESULT_OK            = 0,
};

struct R_EXPORT Thread {
    ThreadFunction      func;
    void*               payload;
    SizeT               uid;
    void*               handle;
    U32                 resultCode;
};


R_EXPORT ErrType createThread(Thread* thread, ThreadFunction startRoutine);
R_EXPORT ErrType resumeThread(Thread* thread);
R_EXPORT ErrType stopThread(Thread* thread);
R_EXPORT ErrType detachThread(Thread* thread);
R_EXPORT ErrType joinThread(Thread* thread);
R_EXPORT ErrType killThread(Thread* thread);

R_EXPORT void* createMutex();
R_EXPORT ErrType lockMutex(void* mutex);
R_EXPORT ErrType unlockMutex(void* mutex);
R_EXPORT ErrType waitMutex(void* mutex, U64 waitTimeMs);
R_EXPORT ErrType destroyMutex(void* mutex);

R_EXPORT ErrType atomicAdd();
R_EXPORT ErrType atomicSub();
} // Recluse