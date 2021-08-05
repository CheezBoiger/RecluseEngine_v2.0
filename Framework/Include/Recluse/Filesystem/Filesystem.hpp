//
#pragma once 

#include "Recluse/Types.hpp"
#include <vector>

namespace Recluse {


struct FileBufferData {
    std::vector<char> buffer;    
};


struct R_EXPORT File {
    File()
        : m_isOpen(false)
        , m_fileHandle(nullptr) { }


    static ErrType readFrom(FileBufferData* pData, const std::string& filePath);
    static ErrType writeTo(FileBufferData* pData, const std::string& filePath);

    ErrType open(const std::string& filePath, char* access);
    void    close();
    ErrType write(void* ptr, U64 szBytes);
    ErrType read(void* ptr, U64 szBytes);
    U64     getFileSz() const;

    B32 isOpen() { return m_isOpen; }

private:
    B32     m_isOpen;
    void*   m_fileHandle;
};


class Filesystem {
public:
    static R_EXPORT std::string getCurrentDir();
    static R_EXPORT std::string getDirectoryFromPath(const std::string& path);

    static R_EXPORT std::string isFile(const std::string& path);

    static R_EXPORT std::string join(const std::string& path0, const std::string& path1);
};
} // Recluse 