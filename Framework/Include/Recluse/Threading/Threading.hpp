//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

// Thread function, used and called as the start routine for creating threads.
typedef U32(*ThreadFunction)(void*);

enum ThreadResultCode 
{
    ThreadResultCode_Unknown       = -999,
    ThreadResultCode_NotReady      = -2,
    ThreadResultCode_Failed        = -1,
    ThreadResultCode_Ok            =  0,
};


enum ThreadState 
{
    ThreadState_NotRunning,
    ThreadState_Running,
    ThreadState_Suspended,
    ThreadState_Idle,
    ThreadState_Unknown
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

R_PUBLIC_API R_OS_CALL ErrType createThread(Thread* thread, ThreadFunction startRoutine);
R_PUBLIC_API R_OS_CALL ErrType resumeThread(Thread* thread);
R_PUBLIC_API R_OS_CALL ErrType stopThread(Thread* thread);
R_PUBLIC_API R_OS_CALL ErrType detachThread(Thread* thread);
R_PUBLIC_API R_OS_CALL ErrType joinThread(Thread* thread);
R_PUBLIC_API R_OS_CALL ErrType killThread(Thread* thread);

R_PUBLIC_API R_OS_CALL Mutex   createMutex(const char* name = nullptr);
R_PUBLIC_API R_OS_CALL ErrType lockMutex(Mutex mutex);
R_PUBLIC_API R_OS_CALL ErrType unlockMutex(Mutex mutex);
R_PUBLIC_API R_OS_CALL ErrType waitMutex(Mutex mutex, U64 waitTimeMs);
R_PUBLIC_API R_OS_CALL ErrType destroyMutex(Mutex mutex);
R_PUBLIC_API R_OS_CALL ErrType tryLockMutex(Mutex mutex);

R_PUBLIC_API R_OS_CALL ErrType atomicAdd();
R_PUBLIC_API R_OS_CALL ErrType atomicSub();
R_PUBLIC_API R_OS_CALL U64     getMainThreadId();
R_PUBLIC_API R_OS_CALL U64     getCurrentThreadId();

R_PUBLIC_API R_OS_CALL Semaphore  createSemaphore();
R_PUBLIC_API R_OS_CALL ErrType    destroySemaphore(Semaphore sema);
R_PUBLIC_API R_OS_CALL ErrType    signalSemaphore(Semaphore sema);
R_PUBLIC_API R_OS_CALL ErrType    waitSemaphore(Semaphore sema);

R_PUBLIC_API R_OS_CALL U64    compareExchange(I64* dest, I64 ex, I64 comp);
R_PUBLIC_API R_OS_CALL I16    compareExchange(I16* dest, I16 ex, I16 comp);
R_PUBLIC_API R_OS_CALL U128   compareExchange(U128* dest, U128 ex, U128 comp);

R_PUBLIC_API R_OS_CALL U32 testAndSet(U32* ptr);

// Causes this thread to sleep for some milliseconds.
R_PUBLIC_API R_OS_CALL ErrType    sleep(U64 milliseconds);

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

class MutexGuard
{
public:
    MutexGuard(const char* debugName) { m_mutex = createMutex(debugName); }
    ~MutexGuard() { destroyMutex(m_mutex); }
    operator Mutex() const { return m_mutex; }
    Mutex* operator*() { return &m_mutex; }
    Mutex& operator&() { return m_mutex; }
private:
    Mutex m_mutex;
};
} // Recluse