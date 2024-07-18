//
#include "Win32Common.hpp"
#include "Win32Runtime.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

#include <algorithm>
#include <vector>

namespace Recluse {


U64 File::getFileSz() const
{
    return GetFileSize((HANDLE)m_fileHandle, NULL);
}


ResultCode File::readFrom(FileBufferData* pFile, const std::string& filePath)
{
    File file;
    ResultCode result = RecluseResult_Ok;

    result = file.open(filePath, "r");

    if (file.isOpen()) 
    {
        DWORD sz        = (DWORD)file.getFileSz();

        pFile->resize(sz+1);
        result = file.read(pFile->data(), pFile->size());
        file.close();
        // Null terminate, as we don't gaurantee one.
        (*pFile)[sz] = '\0';
        R_DEBUG(R_CHANNEL_WIN32, "Read %d bytes of data from file: %s", pFile->size(), filePath.c_str());
    }

    return result;
}


ResultCode File::writeTo(FileBufferData* pFile, const std::string& filePath)
{
    ResultCode result = RecluseResult_Ok;
    File file;
    
    result = file.open(filePath, "w");
    
    if (file.isOpen()) 
    {
        result = file.write(pFile->data(), pFile->size());
        file.close();

        R_DEBUG(R_CHANNEL_WIN32, "Wrote %d bytes of data to file: %s", pFile->size(), filePath.c_str());
    }

    return result;
}


void File::setCursor(U64 szBytes)
{
    LARGE_INTEGER dist = { };
    dist.QuadPart = szBytes;
    LARGE_INTEGER ptr = { };
    SetFilePointerEx((HANDLE)m_fileHandle, dist, &ptr, FILE_BEGIN);
}


U64 File::getCursor()
{
    LARGE_INTEGER ptr = { };
    LARGE_INTEGER dist; 
    dist.QuadPart = 0;
    BOOL result = SetFilePointerEx((HANDLE)m_fileHandle, dist, &ptr, FILE_CURRENT); 
    if (result)
    {
        return ptr.QuadPart;
    }
    return 0ull;
}

typedef struct
{
    FileBufferDataAsync*    pAsyncBuffer;
    std::string             filePath;
    ResultCode                 (*taskFn)       (FileBufferData*, const std::string&);
} FileBufferTemporary;


static ResultCode runFileAsyncTask(void* pData)
{
    R_ASSERT(pData != NULL);

    FileBufferTemporary* pTemporary = reinterpret_cast<FileBufferTemporary*>(pData);
    
    ResultCode result = pTemporary->taskFn(&pTemporary->pAsyncBuffer->data, pTemporary->filePath);

    pTemporary->pAsyncBuffer->isFinished = true;
    
    // Handle the cleanup right after. Our thread upon creation, will pass the payload over, to which after
    // will be destroyed. This means that this thread task will need to be responsible for cleaning up this payload,
    // since it is an unsafe allocation, and memory leaks will suffice.
    delete pTemporary;

    return result;
}
ResultCode File::readFromAsync(FileBufferDataAsync* pBuffer, const std::string& filePath
)
{
    static MemoryPool memPool = MemoryPool(sizeof(Thread) * 64ull);
    static LinearAllocator linAllocator;
    R_ASSERT(pBuffer != NULL);

    Thread thr          = { };
    thr.payload         = new FileBufferTemporary();
    
    {
        FileBufferTemporary* temp   = reinterpret_cast<FileBufferTemporary*>(thr.payload);
        temp->filePath              = filePath;
        temp->pAsyncBuffer          = pBuffer;
        temp->taskFn                = File::readFrom;
    }

    pBuffer->isFinished = false;

    ResultCode error = createThread(&thr, runFileAsyncTask);

    //return error;
    return RecluseResult_NoImpl;
}


ResultCode File::writeToAsync(FileBufferDataAsync* pBuffer, const std::string& filePath)
{
    R_ASSERT(pBuffer != NULL);

    Thread thr = { };
    thr.payload = new FileBufferTemporary();

    {
        FileBufferTemporary* temp   = reinterpret_cast<FileBufferTemporary*>(thr.payload);
        temp->filePath              = filePath;
        temp->pAsyncBuffer          = pBuffer;
        temp->taskFn                = File::writeTo;
    }

    pBuffer->isFinished = false;

    ResultCode error = createThread(&thr, runFileAsyncTask);

    return RecluseResult_NoImpl;
}


std::vector<std::string> Filesystem::split(const std::string& filename)
{
    size_t f = filename.find_last_of("/\\");
    return { filename.substr(0, f), filename.substr(f + 1) };
}


ResultCode File::open(const std::string& filePath, char* access)
{
    if (m_isOpen) 
    {
        R_ERROR(R_CHANNEL_WIN32, "This File is already open...");

        return RecluseResult_Ok;
    }

    DWORD acc = 0;
    DWORD o = OPEN_EXISTING;
    U64 len = strlen(access);

    for (U32 i = 0; i < len; ++i) 
    {
        if (access[i] == 'w') 
        {
            acc |= GENERIC_WRITE;
            o = CREATE_ALWAYS;
        } 
        else if (access[i] == 'r') 
        {
            acc |= GENERIC_READ; 
        } 
        else if (access[i] == '+') 
        {
            acc |= FILE_APPEND_DATA;
        }
    }

    if ((acc & (FILE_APPEND_DATA))) 
    {
        o = OPEN_ALWAYS;
    }
    
    HANDLE handle = CreateFile
                        (
                            filePath.c_str(),
                            acc, 
                            0, 
                            NULL, 
                            o, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL
                        );

    if (handle == INVALID_HANDLE_VALUE) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Failed to open file: %s", filePath.c_str());
        return RecluseResult_Failed;
    }

    m_fileHandle    = (void*)handle;
    m_isOpen        = true;

    return RecluseResult_Ok;
}


void File::close()
{
    if (m_fileHandle) 
    {    
        BOOL closed = CloseHandle((HANDLE)m_fileHandle);
        if (closed) 
        {
            m_fileHandle    = nullptr;
            m_isOpen        = false;
        } 
        else 
        {
            R_ERROR(R_CHANNEL_WIN32, "Failed to close file!");
        }
    }
}


ResultCode File::write(const void* ptr, U64 szBytes)
{
    DWORD numBytesWritten   = 0;
    BOOL isWritten          = WriteFile(m_fileHandle, ptr, (DWORD)szBytes, &numBytesWritten, 0);

    if (!isWritten) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Failed to write to file...");

        return RecluseResult_Failed;
    }

    return RecluseResult_Ok;
}


ResultCode File::read(void* ptr, U64 szBytes)
{
    // Return invalid if we are requesting to read nothing...
    if (szBytes == 0) 
    {
        return RecluseResult_InvalidArgs;
    }

    DWORD bytesRead = 0;
    BOOL isRead     = ReadFile((HANDLE)m_fileHandle, ptr, szBytes, &bytesRead, NULL);

    if (!isRead) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Failed to read to file!");

        return RecluseResult_Failed;
    }

    // zero bytes read means we probably reached end of file...
    if (bytesRead == 0) 
    {
        return RecluseResult_Failed;
    }

    return RecluseResult_Ok;
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


Bool directoryExists(const std::string& dirPath)
{
    DWORD dwAttributes = GetFileAttributes(dirPath.c_str());
    return ((dwAttributes != INVALID_FILE_ATTRIBUTES) && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY));
}


Bool Filesystem::createDirectory(const std::string& directoryPath)
{
    I32 pos = 0;
    do
    {
        pos = directoryPath.find_first_of("\\/", pos + 1);
        if (!CreateDirectory(directoryPath.substr(0, pos).c_str(), NULL))
        {
            DWORD err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS)
            {
                return false;
            }
        }
    } while (pos != std::string::npos);
    return true;
}
} // Recluse