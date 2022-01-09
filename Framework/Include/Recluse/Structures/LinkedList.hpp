// 
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include <initializer_list>

namespace Recluse {


template
    <
        typename T, 
        typename Compare = GenericCompare<T>,
        typename AllocatorType = DefaultAllocator,
        class MemPool = MemoryPool
    >
class LinkedList 
{

    typedef T   LinkedListT;
    typedef T*  LinkedListTPtr;
    typedef T&  LinkedListTRef;
    typedef Compare LinkedListCompare;

    typedef struct LinkedNode 
    {
        const LinkedListT   data;
        struct LinkedNode*  pNext;
    } *PLinkedNode;


    struct Iterator 
    {
        LinkedNode* pNext;

        void iterate()
        { 
            if (pNext) 
            {
                pNext = pNext->pNext;
            } 
        }
    };


public:

    void pushBack(const LinkedListTRef& lh) 
    {
        if (!m_pRoot) 
        {
            m_pRoot = 
        }
    }

    void get() 
    { 
        
    }

    U64 find(const T& obj) 
    {
        
    }


    Iterator* begin() 
    {
        
    }

    Iterator* end()
    {
        return;
    }

    Bool isEmpty() const { return m_totalNodes == 0; }
    SizeT getTotalNodes() const { return m_totalNodes; }

private:
    LinkedListCompare   m_cmp;
    SizeT               m_totalNodes;
    PLinkedNode         m_pRoot;
    PLinkedNode         m_pTail;
};
} // Recluse