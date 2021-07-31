//
#pragma once

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


class Archive {
public:

    Archive(const std::string& filepath)
        : m_filepath(filepath)
        , m_cursor(0ull) { }

    void writeHeader(const std::string& header);
    std::string readHeader();

private:
    File    m_file;
    PtrType m_cursor;
    std::string m_filepath;
};
} // Recluse