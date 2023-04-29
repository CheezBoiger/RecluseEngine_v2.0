//
#pragma once

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


class Archive 
{
public:

    R_PUBLIC_API Archive(const std::string& filepath)
        : m_filepath(filepath)
        , m_cursor(0ull) { }

    void writeHeader(void* dat, SizeT szBytes);
    ResultCode readHeader(SizeT szBytes, void** pOutput);

    ResultCode R_PUBLIC_API write(void* ptr, U64 sz);
    ResultCode R_PUBLIC_API read(void* ptr, U64 sz);

    ResultCode R_PUBLIC_API open(char* access);
    ResultCode R_PUBLIC_API close();

private:
    File    m_file;
    UPtr m_cursor;
    std::string m_filepath;
};
} // Recluse