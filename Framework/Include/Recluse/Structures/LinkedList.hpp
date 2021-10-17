// 
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"

namespace Recluse {


template<typename T, typename Cmp = SimpleCompare<T>>
class LinkedList {

    typedef T   LinkedListT;
    typedef T*  LinkedListTPtr;
    typedef T&  LinkedListTRef;
    typedef Cmp LinkedListCompare;

    typedef struct LinkedNode {
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

private:
    LinkedListCompare m_cmp;
    PLinkedNode m_pRoot;
};
} // Recluse