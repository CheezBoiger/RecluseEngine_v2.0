
#include <iostream>

#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Math/Vector2.hpp"

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

    Log::destroyLoggingSystem();

    return 0;
}