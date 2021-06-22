
#include <iostream>

#include "Core/Logger.hpp"
#include "Core/Messaging.hpp"

using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();

    R_DEBUG("TestFramework", "Testing debugging system.");

    R_DEBUG("TestFramework", "We should be getting proper info %d", 50);
    
    R_DEBUG("TestFramework", "Can we see this message?");

    R_ERR("ERROR", "You are an error message!!");

    Log::destroyLoggingSystem();
    return 0;
}