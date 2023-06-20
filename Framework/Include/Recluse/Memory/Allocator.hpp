// 
#ifndef RECLUSE_ALLOCATOR_HPP
#define RECLUSE_ALLOCATOR_HPP
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


class MemoryPool;


//! Recluse allocation struct. Contains info of the suballocation for the requested data.
typedef struct Allocation 
{
    UPtr    baseAddress;        //< Base address/ starting address of the allocated object.
    U64     sizeBytes;          //< The size of the allocated memory that represents this object.
} *PAllocation, &RAllocation;

//! Recluse Allocator class. This is the abstract class that is used for defining multiple allocation
//! data structures.
class R_PUBLIC_API Allocator 
{
public:
    virtual ~Allocator() { }

    Allocator() : m_totalAllocations(0), m_totalSizeBytes(0), m_usedSizeBytes(0), m_pMemoryBaseAddr(0ull) { }

    //! Allocator mem size and page size (usually 4kb). 
    void initialize(UPtr pBasePtr, U64 sizeBytes) 
    {
        m_totalSizeBytes    = sizeBytes;
        m_pMemoryBaseAddr   = pBasePtr;
        m_usedSizeBytes     = 0;
        onInitialize();
    }

    //! Allocation requirements.
    UPtr allocate(U64 requestSz, U16 alignment) 
    {
        Allocation allocation = { };
        ResultCode err = onAllocate(&allocation, requestSz, alignment);
        if (err == RecluseResult_Ok) 
        {
            m_totalAllocations  += 1;
            m_usedSizeBytes     += allocation.sizeBytes;
        }
        m_lastError = err;
        return allocation.baseAddress;
    }

    void free(UPtr ptr) 
    {
        Allocation alloc = { };
        alloc.baseAddress = ptr;
        alloc.sizeBytes = ~0;
        ResultCode err = onFree(&alloc);
        if (err == RecluseResult_Ok) 
        {
            m_usedSizeBytes     -= alloc.sizeBytes;
            m_totalAllocations  -= 1;
        }
        m_lastError = err;
    }


    // Reset the allocator. This is more colloquially known as Clear().
    void reset() 
    {
        onReset();
        m_usedSizeBytes     = 0;
        m_totalAllocations  = 0;
        m_lastError = RecluseResult_Ok;
    }

    void cleanUp() 
    {
        onCleanUp();
        m_totalAllocations  = 0;
        m_totalSizeBytes    = 0;
        m_usedSizeBytes     = 0;
        m_pMemoryBaseAddr   = 0ull;
        m_lastError = RecluseResult_Ok;
    }

    inline U64 getTotalSizeBytes() const 
    { 
        return m_totalSizeBytes; 
    }

    inline U64 getUsedSizeBytes() const 
    { 
        return m_usedSizeBytes; 
    }

    inline U64 getTotalAllocations() const 
    { 
        return m_totalAllocations; 
    }

    inline UPtr getBaseAddr() 
    { 
        return m_pMemoryBaseAddr; 
    }

    ResultCode getLastError() const { return m_lastError; }

protected:

    virtual ResultCode onInitialize() = 0;
    virtual ResultCode onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) = 0;
    virtual ResultCode onFree(Allocation* pOutput) = 0;
    virtual ResultCode onReset() = 0;
    virtual ResultCode onCleanUp() = 0;

private:
    U64     m_totalSizeBytes;
    UPtr    m_pMemoryBaseAddr;
    U64     m_usedSizeBytes;
    U64     m_totalAllocations;
    ResultCode m_lastError = RecluseResult_Ok;
};


class MallocAllocator : public Allocator 
{
public:
    MallocAllocator() { }
    virtual ~MallocAllocator() { }

    virtual ResultCode onInitialize() override 
    { 
        return RecluseResult_Ok; 
    }

    virtual ResultCode onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override 
    {
        U64 offset = alignment - 1 + sizeof(void*);
        U64 neededSzBytes = requestSz + offset;
        void* ptr = malloc(neededSzBytes);
        void** ptrr = (void**)(((UPtr)(ptr) + offset) & ~(alignment - 1));
        ptrr[-1] = ptr;
        pOutput->baseAddress = (U64)(void*)ptrr;
        pOutput->sizeBytes = requestSz;
        return RecluseResult_Ok;
    }

    virtual ResultCode onFree(Allocation* pOutput) override
    {
        void** ptrr = (void**)pOutput->baseAddress;
        ::free(ptrr[-1]);
        return RecluseResult_Ok;
    }

    virtual ResultCode onReset() override { return RecluseResult_Ok; }
    virtual ResultCode onCleanUp() override { return RecluseResult_Ok; }
};
} // Recluse


// Operator overload for placement new.
// This will allow us to overload the placement new operator in c++, to utilize our custom memory allocations.:
// ex. 
//          Object* pObj = new (allocator) Object();
//
R_PUBLIC_API void*   operator new (size_t sizeBytes, Recluse::Allocator* alloc);

// Operator overload for deleting allocated pointers.
// This is a helpful function, instead of having to all individually the object allocator, and performing a bunch of
// stuff...
//
R_PUBLIC_API void    operator delete (void* ptr, Recluse::Allocator* alloc);
#endif // RECLUSE_ALLOCATOR_HPP