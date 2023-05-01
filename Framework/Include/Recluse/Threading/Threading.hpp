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

constexpr U64 kInfiniteMs = ~0ull;

R_PUBLIC_API R_OS_CALL ResultCode createThread(Thread* thread, ThreadFunction startRoutine);
R_PUBLIC_API R_OS_CALL ResultCode resumeThread(Thread* thread);
R_PUBLIC_API R_OS_CALL ResultCode stopThread(Thread* thread);
R_PUBLIC_API R_OS_CALL ResultCode detachThread(Thread* thread);
R_PUBLIC_API R_OS_CALL ResultCode joinThread(Thread* thread);
R_PUBLIC_API R_OS_CALL ResultCode killThread(Thread* thread);

R_PUBLIC_API R_OS_CALL Mutex   createMutex(const char* name = nullptr);
R_PUBLIC_API R_OS_CALL ResultCode lockMutex(Mutex mutex, U64 waitMs = kInfiniteMs);
R_PUBLIC_API R_OS_CALL ResultCode unlockMutex(Mutex mutex);
R_PUBLIC_API R_OS_CALL ResultCode waitMutex(Mutex mutex, U64 waitTimeMs);
R_PUBLIC_API R_OS_CALL ResultCode destroyMutex(Mutex mutex);
R_PUBLIC_API R_OS_CALL ResultCode tryLockMutex(Mutex mutex);

R_PUBLIC_API R_OS_CALL ResultCode atomicAdd();
R_PUBLIC_API R_OS_CALL ResultCode atomicSub();
R_PUBLIC_API R_OS_CALL U64     getMainThreadId();
R_PUBLIC_API R_OS_CALL U64     getCurrentThreadId();

R_PUBLIC_API R_OS_CALL Semaphore  createSemaphore(const char* name = nullptr);
R_PUBLIC_API R_OS_CALL ResultCode    destroySemaphore(Semaphore sema);
R_PUBLIC_API R_OS_CALL ResultCode    signalSemaphore(Semaphore sema);
R_PUBLIC_API R_OS_CALL ResultCode    waitSemaphore(Semaphore sema);

R_PUBLIC_API R_OS_CALL U64    compareExchange(I64* dest, I64 ex, I64 comp);
R_PUBLIC_API R_OS_CALL I16    compareExchange(I16* dest, I16 ex, I16 comp);
R_PUBLIC_API R_OS_CALL U128   compareExchange(U128* dest, U128 ex, U128 comp);

R_PUBLIC_API R_OS_CALL Bool testAndSet(U32* ptr, U32 offset);

// Causes this thread to sleep for some milliseconds.
R_PUBLIC_API R_OS_CALL ResultCode    sleep(U64 milliseconds);

// C++ RAII locking mechanism within a scope.
// Intended for scope locking mutexes.
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
    MutexGuard(const char* debugName = nullptr) { m_mutex = createMutex(debugName); }
    ~MutexGuard() { destroyMutex(m_mutex); }
    operator Mutex() const { return m_mutex; }
    Mutex* operator*() { return &m_mutex; }
    Mutex& operator&() { return m_mutex; }
private:
    Mutex m_mutex;
};


// Critical section is a way to internally handle concurrency. 
// Microsoft may attempt to handle critical code sections in user-mode in order to 
// prevent kernel level calls, which would be slower. Linux simply uses mutexes 
// and spinlocks, which can provide about similar performance as well.
class R_PUBLIC_API CriticalSection
{
public:
    CriticalSection()
        : m_section(nullptr)
    { }
    ~CriticalSection()
    { if (m_section) release(); m_section = nullptr; }

    ResultCode initialize();
    ResultCode release();

    R_OS_CALL ResultCode enter();

    // Returns Ok if the critical section is owned by this thread. Returns fail, if 
    // the attempt fails.
    R_OS_CALL ResultCode tryEnter();

    R_OS_CALL ResultCode leave();

private:
    void* m_section;
};

// C++ RAII critical section mechanism used for handling enter and exit
// calls to the critical section. 
class R_PUBLIC_API ScopedCriticalSection
{
public:
    ScopedCriticalSection(CriticalSection& cs)
        : m_cs(cs)
    {
        m_cs.enter();
    }

    ~ScopedCriticalSection()
    {
        m_cs.leave();
    }
private:
    CriticalSection& m_cs;
};
} // Recluse