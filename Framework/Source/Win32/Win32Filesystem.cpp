//
#include "Win32Common.hpp"
#include "Win32Runtime.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include <algorithm>

namespace Recluse {


U64 File::getFileSz() const
{
    return GetFileSize((HANDLE)m_fileHandle, NULL);
}


ErrType File::readFrom(FileBufferData* pFile, const std::string& filePath)
{
    File file;
    ErrType result = REC_RESULT_OK;

    result = file.open(filePath, "r");

    if (file.isOpen()) {
        DWORD sz        = file.getFileSz();

        pFile->buffer.resize(sz);
        result = file.read(pFile->buffer.data(), pFile->buffer.size());
        file.close();
        
        R_DEBUG(R_CHANNEL_WIN32, "Read %d bytes of data from file: %s", pFile->buffer.size(), filePath.c_str());
    }

    return result;
}


ErrType File::writeTo(FileBufferData* pFile, const std::string& filePath)
{
    ErrType result = REC_RESULT_OK;
    File file;
    
    result = file.open(filePath, "w");
    
    if (file.isOpen()) {
        result = file.write(pFile->buffer.data(), pFile->buffer.size());
        file.close();

        R_DEBUG(R_CHANNEL_WIN32, "Wrote %d bytes of data to file: %s", pFile->buffer.size(), filePath.c_str());
    }

    return result;
}


ErrType File::open(const std::string& filePath, char* access)
{
    if (m_isOpen) {

        R_ERR(R_CHANNEL_WIN32, "This File is already open...");

        return REC_RESULT_OK;

    }

    DWORD acc = 0;
    DWORD o = OPEN_EXISTING;
    U64 len = strlen(access);

    for (U32 i = 0; i < len; ++i) {
        if (access[i] == 'w') {
            acc |= GENERIC_WRITE;
            o = CREATE_ALWAYS;
        } else if (access[i] == 'r') {
            acc |= GENERIC_READ; 
        } else if (access[i] == '+') {
            acc |= FILE_APPEND_DATA;
        }
    }

    if ((acc & (FILE_APPEND_DATA))) {
        o = OPEN_ALWAYS;
    }
    
    HANDLE handle = CreateFile(filePath.c_str(),
        acc, 
        0, 
        NULL, 
        o, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        R_ERR(R_CHANNEL_WIN32, "Failed to open file: %s", filePath.c_str());
        return REC_RESULT_FAILED;
    }

    m_fileHandle    = (void*)handle;
    m_isOpen        = true;

    return REC_RESULT_OK;
}


void File::close()
{
    if (m_fileHandle) {
        
        BOOL closed = CloseHandle((HANDLE)m_fileHandle);
        if (closed) {
            m_fileHandle    = nullptr;
            m_isOpen        = false;
        } else {
            R_ERR(R_CHANNEL_WIN32, "Failed to close file!");
        }
    }
}


ErrType File::write(void* ptr, U64 szBytes)
{
    DWORD numBytesWritten   = 0;
    BOOL isWritten          = WriteFile(m_fileHandle, ptr, szBytes, &numBytesWritten, 0);

    if (!isWritten) {
    
        R_ERR(R_CHANNEL_WIN32, "Failed to write to file...");

        return REC_RESULT_FAILED;

    }

    return REC_RESULT_OK;
}


ErrType File::read(void* ptr, U64 szBytes)
{
    DWORD bytesRead = 0;
    BOOL isRead     = ReadFile((HANDLE)m_fileHandle, ptr, szBytes, &bytesRead, NULL);

    if (!isRead) {

        R_ERR(R_CHANNEL_WIN32, "Failed to read to file!");

        return REC_RESULT_FAILED;

    }

    return REC_RESULT_OK;
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