// 
#ifndef RECLUSE_ALLOCATOR_HPP
#define RECLUSE_ALLOCATOR_HPP
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


class MemoryPool;


//! Recluse allocation struct. Contains info of the suballocation for the requested data.
typedef struct Allocation 
{
    PtrType baseAddress;        //< Base address/ starting address of the allocated object.
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
    void initialize(PtrType pBasePtr, U64 sizeBytes) 
    {
        m_totalSizeBytes    = sizeBytes;
        m_pMemoryBaseAddr   = pBasePtr;
        m_usedSizeBytes     = 0;
        onInitialize();
    }

    //! Allocation requirements.
    ErrType allocate(Allocation* pOutput, U64 requestSz, U16 alignment) 
    {
        ErrType err = onAllocate(pOutput, requestSz, alignment);
        if (err == R_RESULT_OK) 
        {
            m_totalAllocations  += 1;
            m_usedSizeBytes     += pOutput->sizeBytes;
        }

        return err;
    }

    ErrType free(Allocation* pOutput) 
    {
        ErrType err = onFree(pOutput);
        if (err == R_RESULT_OK) 
        {
            m_usedSizeBytes     -= pOutput->sizeBytes;
            m_totalAllocations  -= 1;
        }
    
        return err;
    }


    // Reset the allocator. This is more colloquially known as Clear().
    void reset() 
    {
        onReset();
        m_usedSizeBytes     = 0;
        m_totalAllocations  = 0;
    }

    void cleanUp() 
    {
        onCleanUp();
        m_totalAllocations  = 0;
        m_totalSizeBytes    = 0;
        m_usedSizeBytes     = 0;
        m_pMemoryBaseAddr   = 0ull;
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

    inline PtrType getBaseAddr() 
    { 
        return m_pMemoryBaseAddr; 
    }

protected:

    virtual ErrType onInitialize() = 0;
    virtual ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) = 0;
    virtual ErrType onFree(Allocation* pOutput) = 0;
    virtual ErrType onReset() = 0;
    virtual ErrType onCleanUp() = 0;

private:
    U64     m_totalSizeBytes;
    PtrType m_pMemoryBaseAddr;
    U64     m_usedSizeBytes;
    U64     m_totalAllocations;
};


class DefaultAllocator : public Allocator 
{
public:

    virtual ErrType onInitialize() override 
    { 
        return R_RESULT_OK; 
    }

    virtual ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override 
    {
        return R_RESULT_OK;
    }
};
} // Recluse


// Operator overload for placement new.
// This will allow us to overload the placement new operator in c++, to utilize our custom memory allocations.:
// ex. 
//          Object* pObj = new (allocator) Object();
//
R_PUBLIC_API void*   operator new (size_t sizeBytes, Recluse::Allocator* alloc, Recluse::Allocation* pOutput);
R_PUBLIC_API void*   operator new (size_t sizeBytes, Recluse::Allocator* alloc);

// Operator overload for deleting allocated pointers.
// This is a helpful function, instead of having to all individually the object allocator, and performing a bunch of
// stuff...
//
R_PUBLIC_API void    operator delete (void* ptr, Recluse::Allocator* alloc);
#endif // RECLUSE_ALLOCATOR_HPP