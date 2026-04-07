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


ResultCode File::readFrom(FileBufferData* pFile, const std::string& filePath, Flags flags)
{
    File file;
    ResultCode result = RecluseResult_Ok;

    result = file.open(filePath, "r");

    if (file.isOpen()) 
    {
        DWORD sz        = (DWORD)file.getFileSz();

        if (flags & NullTerminate)
            sz += 1;

        pFile->resize(sz);
        result = file.read(pFile->data(), pFile->size());
        file.close();

        if (flags & NullTerminate)
        {
            // Null terminate, as we don't gaurantee one.
            (*pFile)[sz-1] = '\0';
        }
        R_DEBUG(R_CHANNEL_WIN32, "Read %d bytes of data from file: %s", pFile->size(), filePath.c_str());
    }

    return result;
}


ResultCode File::writeTo(const char* pData, U64 sizeBytes, const std::string& filePath, Flags flags)
{
    ResultCode result = RecluseResult_Ok;
    File file;

    result = file.open(filePath, "w", flags);

    if (file.isOpen())
    {
        result = file.write(pData, sizeBytes);
        file.close();

        R_DEBUG(R_CHANNEL_WIN32, "Wrote %d bytes of data to file: %s", sizeBytes, filePath.c_str());
    }

    return result;
}


ResultCode File::writeTo(FileBufferData* pFile, const std::string& filePath, Flags flags)
{
    return File::writeTo(pFile->data(), pFile->size(), filePath, flags);
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
    File::Flags             flags;
    ResultCode                 (*taskFn)       (FileBufferData*, const std::string&, File::Flags);
} FileBufferTemporary;


static ResultCode runFileAsyncTask(void* pData)
{
    R_ASSERT(pData != NULL);

    FileBufferTemporary* pTemporary = reinterpret_cast<FileBufferTemporary*>(pData);
    
    ResultCode result = pTemporary->taskFn(&pTemporary->pAsyncBuffer->data, pTemporary->filePath, pTemporary->flags);

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


ResultCode File::writeToAsync(FileBufferDataAsync* pBuffer, const std::string& filePath, Flags flags)
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


Bool directoryExists(const std::string& dirPath)
{
    DWORD dwAttributes = GetFileAttributes(dirPath.c_str());
    return ((dwAttributes != INVALID_FILE_ATTRIBUTES) && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY));
}


ResultCode checkAndMakeFilePath(const std::string& filePath)
{
    // 1. Iterate through the string to create directories
    for (size_t i = 0; i < filePath.length(); i++) 
    {
        if (filePath[i] == L'\\' || filePath[i] == L'/') 
        {
            std::string subPath = filePath.substr(0, i);

            // Skip the drive letter (e.g., "C:")
            if (subPath.length() <= 2 && subPath.find(":") != std::wstring::npos) 
            {
                continue;
            }

            // Check existence before attempting creation
            if (!subPath.empty() && !directoryExists(subPath)) 
            {
                if (!CreateDirectory(subPath.c_str(), NULL)) {
                    // Handle error if needed (e.g., permissions)
                    return RecluseResult_Failed;
                }
            }
        }
    }
    return RecluseResult_Ok;
}


ResultCode File::open(const std::string& filePath, char* access, Flags flags)
{
    if (m_isOpen) 
    {
        R_ERROR(R_CHANNEL_WIN32, "This File is already open...");

        return RecluseResult_Ok;
    }

    if (flags & File::Config::Recursive)
    {
        checkAndMakeFilePath(filePath);
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


Bool Filesystem::createDirectory(const std::string& directoryPath)
{
    I32 pos = 0;
    do
    {
        pos = static_cast<i32>(directoryPath.find_first_of("\\/", static_cast<sizet>(pos + 1)));
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


std::string File::extension(const std::string& filePath)
{
    std::string ext;
    if (size_t off = filePath.find_last_of('.'))
    {
        ext = filePath.substr(off, filePath.size());
    }
    return ext;
}
} // Recluse