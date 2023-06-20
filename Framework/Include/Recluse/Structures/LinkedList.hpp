// 
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include <initializer_list>

namespace Recluse {


template<typename T, typename Compare = CompareLess<T>, typename TypeEqual = CompareEqual<T>, typename AllocatorType = MallocAllocator>
class LinkedList 
{

    typedef T           Type;
    typedef T*          TypePointer;
    typedef T&          TypeReference;
    typedef const T&    ConstantTypeReference;
    typedef const T*    ConstantTypePointer;
    typedef Compare     LinkedListCompare;

    typedef struct Node 
    {
        struct Node*  next;
        Type          data;
    } *PNode;


    struct Iterator 
    {
        Node* pNext;

        void iterate()
        { 
            if (pNext) 
            {
                pNext = pNext->pNext;
            } 
        }
    };

    typedef Iterator*       IteratorPointer;
    typedef Iterator&       IteratorRefrerence;
    typedef const Iterator* ConstantIteratorPointer;
    typedef const Iterator& ConstantIteratorReference;


public:
    LinkedList()
        : m_root(nullptr)
        , m_tail(nullptr)
        , m_cmp(Compare())
        , m_equal(TypeEqual())
        , m_totalNodes(0)
    { }

    void            pushBack(ConstantTypeReference data);
    Type            pop();
    void            get(SizeT idx = 0);
    Iterator*       find(ConstantTypeReference data);


    Iterator*       begin();

    Iterator*       end();

    SizeT           size() const { return m_totalNodes; }
    Bool            empty() const { return (size() == 0); }

private:
    LinkedListCompare   m_cmp;
    TypeEqual           m_equal;
    SizeT               m_totalNodes;
    PNode               m_root;
    PNode               m_tail;
};
} // Recluse