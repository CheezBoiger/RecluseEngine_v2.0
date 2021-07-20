//
#pragma once 

#include "Recluse/Types.hpp"
#include <vector>

namespace Recluse {


struct File {
    std::vector<char> data;

    static R_EXPORT ErrType readFrom(File* pFile, const std::string& filePath);
    static R_EXPORT ErrType writeTo(File* pFile, const std::string& filePath);
};


class Filesystem {
public:
    static R_EXPORT std::string getCurrentDir();
    static R_EXPORT std::string getDirectoryFromPath(const std::string& path);
};
} // Recluse 