//
#pragma once 

#include "Recluse/Types.hpp"
#include <vector>

namespace Recluse {


typedef std::vector<char> FileBufferData;

struct FileBufferDataAsync
{
    FileBufferData data;
    Bool isFinished;
};

// File is an abstract construct of the operating system files.
// Normally used to write to / read from file instances in the filesystem.
struct R_PUBLIC_API File 
{
    File()
        : m_isOpen(false)
        , m_fileHandle(nullptr) { }

    // Quick read/write calls to files, which will not open them any longer than needed in order 
    // to read/write to a buffer.

    // Read from a filepath.
    static ResultCode readFrom(FileBufferData* pData, const std::string& filePath);

    // Write to a filepath.
    static ResultCode writeTo(FileBufferData* pData, const std::string& filePath);

    // Read from a filepath asyncronously. This will immediately return back to the caller.
    // Updates the FileBufferDataAsync object when the separate run finishes.
    static ResultCode readFromAsync(FileBufferDataAsync* pBuffer, const std::string& filePath);

    // Write to a filepath asyncronously. This will immediately return back to the caller.
    // Updates the FileBufferDataAsync object when the separate run finishes.
    static ResultCode writeToAsync(FileBufferDataAsync* pBuffer, const std::string& filePath);

    // Opens the file with the given access permissions.
    ResultCode open(const std::string& filePath, char* access);

    // Closes the file handle, this is required, otherwise the handle remains open!
    void    close();

    // Writes to the file handle, you must call open() in order to write!
    ResultCode write(const void* ptr, U64 szBytes);

    // Reads from the file handle, you must call open() in order to write!
    ResultCode read(void* ptr, U64 szBytes);

    // Get the total file size!
    U64     getFileSz() const;

    // Check if the file object is open.
    B32 isOpen() { return m_isOpen; }

    // Set the file cursor from the start.
    void setCursor(U64 bytes);

    U64 getCursor();

private:
    B32     m_isOpen;
    void*   m_fileHandle;
};


namespace Filesystem {

// Get the current working directory.
extern R_PUBLIC_API std::string getCurrentDir();

// Get the directoy from the given path.
extern R_PUBLIC_API std::string getDirectoryFromPath(const std::string& path);

// Check if the path is a path to a file. If not, return false.
extern R_PUBLIC_API Bool isFile(const std::string& path);

// Join to paths together.
extern R_PUBLIC_API std::string join(const std::string& path0, const std::string& path1);


extern R_PUBLIC_API Bool pathExists(const std::string& path);

extern R_PUBLIC_API std::vector<std::string> split(const std::string& filename);

extern R_PUBLIC_API Bool createDirectory(const std::string& directoryPath);
} // Filesystem.
} // Recluse 