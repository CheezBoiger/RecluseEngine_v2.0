//
#pragma once

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


class Archive {
public:

    R_EXPORT Archive(const std::string& filepath)
        : m_filepath(filepath)
        , m_cursor(0ull) { }

    void writeHeader(const std::string& header);
    std::string readHeader();

    ErrType R_EXPORT write(void* ptr, U64 sz);
    ErrType R_EXPORT read(void* ptr, U64 sz);

    ErrType R_EXPORT open(char* access);
    ErrType R_EXPORT close();

private:
    File    m_file;
    PtrType m_cursor;
    std::string m_filepath;
};
} // Recluse