// 
#include "Core/Logger.hpp"

#include "Core/Messaging.hpp"
#include "Core/Logging/LogFramework.hpp"

namespace Recluse {


static LoggingDatabase* pDatabase;

Log::~Log()
{
    R_ASSERT(pDatabase != NULL);
    pDatabase->store(*this);
}


void LoggingDatabase::store(const Log& log)
{
    
}
} // Recluse