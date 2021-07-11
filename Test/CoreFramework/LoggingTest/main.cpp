
#include <iostream>

#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Threading/Threading.hpp"

#include <vector>

using namespace Recluse;

ErrType printHello(void* data)
{
    I32 uid = *(I32*)data;

    R_DEBUG("TestFramework", "Testing debugging system. uid=%d", uid);

    R_DEBUG("TestFramework", "We should be getting proper info %d uid=%d", 50, uid);
    
    R_INFO("INFO", "Just a normal info message. uid=%d", uid);

    R_DEBUG("TestFramework", "Can we see this message? uid=%d", uid);

    R_ERR("ERROR", "You are an error message!! uid=%d", uid);

    R_WARN("WARN", "Issuing a warning...!! uid=%d", uid);

    R_VERBOSE("VERBOSE", "I Am a verbose message!!!!!?!! uid=%d", uid);
    
    R_TRACE("TRACE", "I am a trace message!??!?!?!?!uid=%d", uid);
    
    U32 count = 0;
    while (count < 10000) {
        R_TRACE("Thread", "I am %d", uid);    
        count++;
    }

    return REC_RESULT_OK;
}

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();

    std::vector<Thread> threads(5);
    std::vector<I32> uids(5);
    
    for (U32 i = 0; i < threads.size(); ++i) {
        uids[i] = i;
        threads[i].payload = &uids[i]; 
        createThread(&threads[i], printHello);
    
    }

    Log::destroyLoggingSystem();

    return 0;
}