//
#include "Core/Win32/Win32Common.hpp"
#include "Core/Types.hpp"

namespace Recluse {


B32 loadLibrary(char* libPath)
{
    HMODULE library = LoadLibrary(libPath);
    
    return library != NULL;
}
} // Recluse  