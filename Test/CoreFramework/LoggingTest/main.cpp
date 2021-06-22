
#include <iostream>

#include "Core/Logger.hpp"
#include "Core/Messaging.hpp"

using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();

    R_DEBUG("TestFramework", "Testing debugging system.");

    R_DEBUG("TestFramework", "We should be getting proper info %d", 50);
    
    R_INFO("INFO", "Just a normal info message.");

    R_DEBUG("TestFramework", "Can we see this message?");

    R_ERR("ERROR", "You are an error message!!");

    R_WARN("WARN", "Issuing a warning...!!");

    R_VERBOSE("VERBOSE", "I Am a verbose message!!!!!?!!");
    
    R_TRACE("TRACE", "I am a trace message!??!?!?!?!");

    Log::destroyLoggingSystem();
    return 0;
}