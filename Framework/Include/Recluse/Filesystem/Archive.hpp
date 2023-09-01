//
#pragma once

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


class Archive 
{
public:

    R_PUBLIC_API Archive(const std::string& filepath, char* access)
        : m_filepath(filepath)
        , m_cursor(0ull) 
    {
        ResultCode result = openFile(access); 
    }

    virtual ~Archive() 
    {
        if (isOpen())
            close(); 
    }

    ResultCode R_PUBLIC_API write(void* ptr, U64 sz);
    ResultCode R_PUBLIC_API read(void* ptr, U64 sz);

    ResultCode R_PUBLIC_API close();

    Bool isOpen() { return m_file.isOpen(); }

protected:
    ResultCode R_PUBLIC_API openFile(char* access);

private:
    File            m_file;
    UPtr            m_cursor;
    std::string     m_filepath;
};


class ArchiveReader : public Archive
{
public:
    ArchiveReader(const std::string& filePath)
        : Archive(filePath, "r") { }
};


class ArchiveWriter : public Archive
{
public:
    ArchiveWriter(const std::string& filePath)
        : Archive(filePath, "w") { }
};
} // Recluse