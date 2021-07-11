//
#include "Win32/Win32Common.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


B32 loadLibrary(char* libPath)
{
    R_DEBUG("Win32", "Loading library: %s", libPath);

    HMODULE library = LoadLibrary(libPath);
    
    return library != NULL;
}
} // Recluse  