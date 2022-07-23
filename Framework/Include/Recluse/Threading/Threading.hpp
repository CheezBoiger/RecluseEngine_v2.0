//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

// Thread function, used and called as the start routine for creating threads.
typedef U32(*ThreadFunction)(void*);

enum ThreadResultCode 
{
    THREAD_RESULT_UNKNOWN       = -999,
    THREAD_RESULT_NOT_READY     = -2,
    THREAD_RESULT_FAILED        = -1,
    THREAD_RESULT_OK            = 0,
};


enum ThreadState 
{
    THREAD_STATE_NOT_RUNNING,
    THREAD_STATE_RUNNING,
    THREAD_STATE_SUSPENDED,
    THREAD_STATE_IDLE,
    THREAD_STATE_UNKNOWN
};


struct R_PUBLIC_API Thread 
{
    ThreadFunction      func;
    void*               payload;
    SizeT               uid;
    void*               handle;
    U32                 resultCode;
    U32                 threadState;
};


typedef void* Mutex;
typedef void* Cond;
typedef void* Semaphore;


namespace MutexValue {

constexpr Mutex kNull = nullptr;

} // MutexVal

R_PUBLIC_API ErrType createThread(Thread* thread, ThreadFunction startRoutine);
R_PUBLIC_API ErrType resumeThread(Thread* thread);
R_PUBLIC_API ErrType stopThread(Thread* thread);
R_PUBLIC_API ErrType detachThread(Thread* thread);
R_PUBLIC_API ErrType joinThread(Thread* thread);
R_PUBLIC_API ErrType killThread(Thread* thread);

R_PUBLIC_API Mutex   createMutex(const char* name = nullptr);
R_PUBLIC_API ErrType lockMutex(Mutex mutex);
R_PUBLIC_API ErrType unlockMutex(Mutex mutex);
R_PUBLIC_API ErrType waitMutex(Mutex mutex, U64 waitTimeMs);
R_PUBLIC_API ErrType destroyMutex(Mutex mutex);
R_PUBLIC_API ErrType tryLockMutex(Mutex mutex);

R_PUBLIC_API ErrType atomicAdd();
R_PUBLIC_API ErrType atomicSub();
R_PUBLIC_API U64     getMainThreadId();
R_PUBLIC_API U64     getCurrentThreadId();

R_PUBLIC_API Semaphore  createSemaphore();
R_PUBLIC_API ErrType    destroySemaphore(Semaphore sema);
R_PUBLIC_API ErrType    signalSemaphore(Semaphore sema);
R_PUBLIC_API ErrType    waitSemaphore(Semaphore sema);

R_PUBLIC_API U64    compareExchange(I64* dest, I64 ex, I64 comp);
R_PUBLIC_API I16    compareExchange(I16* dest, I16 ex, I16 comp);
R_PUBLIC_API U128   compareExchange(U128* dest, U128 ex, U128 comp);

R_PUBLIC_API U32 testAndSet(U32* ptr);

// Causes this thread to sleep for some milliseconds.
R_PUBLIC_API ErrType    sleep(U64 milliseconds);

// C++ RAII locking mechanism within a scope.
class R_PUBLIC_API ScopedLock 
{
public:
    volatile ScopedLock(Mutex mutex) 
        : m_mut(mutex)
    {
        lockMutex(m_mut); 
    }

    ~ScopedLock() 
    {
        unlockMutex(m_mut); 
    }
private:
    Mutex m_mut;

    ScopedLock(const ScopedLock&)   = delete;
    ScopedLock(ScopedLock&&)        = delete;
};
} // Recluse