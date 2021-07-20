//
#include "Win32Common.hpp"
#include "Win32Runtime.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include <algorithm>

namespace Recluse {


ErrType File::readFrom(File* pFile, const std::string& filePath)
{
    HANDLE fileH = CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileH == INVALID_HANDLE_VALUE) {

        R_ERR(R_CHANNEL_WIN32, "Count not open filepath: %s", filePath.c_str());

        return REC_RESULT_FAILED;

    }

    DWORD sz        = GetFileSize(fileH, NULL);
    DWORD bytesRead = 0;

    // Add one for null terminator.
    pFile->data.resize(sz);

    ReadFile(fileH, pFile->data.data(), sz, &bytesRead, NULL);
    CloseHandle(fileH);

    R_DEBUG(R_CHANNEL_WIN32, "Read %d bytes of data from file: %s", bytesRead, filePath.c_str());

    return REC_RESULT_OK;
}


ErrType File::writeTo(File* pFile, const std::string& filePath)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


std::string Filesystem::getCurrentDir()
{
    char buffer[MAX_PATH];

    GetModuleFileName(NULL, buffer, MAX_PATH);

    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    std::string currPath = std::string(buffer).substr(0, pos);
    std::replace(currPath.begin(), currPath.end(), '\\', '/'); 

    return currPath;
}


std::string Filesystem::getDirectoryFromPath(const std::string& path)
{
    std::string p = path;
    std::replace(p.begin(), p.end(), '\\', '/');
    
    size_t pos = p.find_last_of('/');
    
    return p.substr(0, pos);
}
} // Recluse