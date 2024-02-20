
#include <iostream>

#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Matrix22.hpp"

#include "Recluse/Algorithms/Mergesort.hpp"
#include "Recluse/Structures/RBTree.hpp"
#include "Recluse/Memory/BuddyAllocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Utility.hpp"
#include <vector>

using namespace Recluse;
using namespace Recluse::Math;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    
    Float2 a = Float2( 1.0f,  1.0f);    
    Float2 b = Float2(-1.0f, -1.0f);

    Float2 c0 = a * b;
    R_TRACE("TEST", "a * b = (%f, %f)", c0.x, c0.y);
    Float2 d = a / b;
    R_TRACE("TEST", "a / b = (%f, %f)", d.x, d.y);
    Float2 e = a + b;
    R_TRACE("TEST", "a + b = (%f, %f)", e.x, e.y);
    Float2 f = a - b;
    R_TRACE("TEST", "a - b = (%f, %f)", f.x, f.y);

    Matrix22 m = { 1, 0, 0, 1 };

    Log(LogType_Error, "Test") << m;

    MemoryArena arena(R_KB(8));
    BuddyAllocator alloc;
    alloc.initialize(arena.getBaseAddress(), arena.getTotalSizeBytes());
    RBTree<I32, CompareLess<I32>, CompareEqual<I32>, BuddyAllocator> tree(alloc);
    tree.insert(4);
    tree.insert(1);
    tree.insert(5);
    tree.insert(12);
    tree.insert(60);
    tree.insert(-1);
    tree.insert(45);
    tree.insert(20);
    tree.insert(10);
    tree.insert(2);
    tree.insert(70);
    tree.insert(100);
    tree.insert(500);
    tree.insert(200);
    tree.insert(450);
    std::string text = tree.stringify();
    R_TRACE("Red-Black Tree", "\n%s", text.c_str());

    tree.remove(60);
    //tree.remove(4);
    tree.remove(12);
    tree.remove(2);
    tree.remove(70);
    tree.remove(-1);
    //tree.remove(45);
    tree.remove(20);
    tree.remove(10);
    tree.remove(100);
    tree.remove(500);
    tree.remove(200);
    tree.remove(450);
    tree.remove(1);
    tree.remove(5);
    text = tree.stringify();
    R_TRACE("Red-Black Tree", "\n%s", text.c_str());

    Math::Color3 color0(255, 255, 255);
    Math::Color3 color1(0, 255, 0);
    U32 len = Math::dist(color0, color1);
    R_TRACE("Colors", "distance: %d", len); 

    int items[10] = { 0, 5, 3, 2, 9, 2, -2, 6, 8, 54 };
    mergeSort<int, CompareGreater<int>>(items, 10, nullptr, CompareGreater<int>());
    Log::destroyLoggingSystem();

    return 0;
}