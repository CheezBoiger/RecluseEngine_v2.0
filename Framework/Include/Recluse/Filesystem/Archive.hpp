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

    void writeHeader(const std::string& header);
    std::string readHeader();

    ErrType R_PUBLIC_API write(void* ptr, U64 sz);
    ErrType R_PUBLIC_API read(void* ptr, U64 sz);

    ErrType R_PUBLIC_API open(char* access);
    ErrType R_PUBLIC_API close();

private:
    File    m_file;
    PtrType m_cursor;
    std::string m_filepath;
};
} // Recluse