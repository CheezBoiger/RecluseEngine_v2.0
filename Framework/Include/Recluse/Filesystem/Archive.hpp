//
#pragma once

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


// Archive is a wrapper object over file reader. It acts as a recorder of 
// data, to which it stores into filepath. Simpler functionality over naked writes/reads.
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

    // Write to file with pointer to data, and the size of the data.
    // Returns RecluseResult_Ok if the write was succesful.
    ResultCode R_PUBLIC_API write(const void* ptr, U64 sz);

    // Reads the file, passing information to input memory, and determines how much to read,
    // from given size bytes. Returns RecluseResult_Ok, if the read operation was successful.
    // 
    ResultCode R_PUBLIC_API read(void* ptr, U64 sz);

    // Closes the archive file. This effectively invalidates the handle to the filepath.
    ResultCode R_PUBLIC_API close();

    // Checks if the file is still open for reading/writing.
    Bool isOpen() { return m_file.isOpen(); }

protected:
    ResultCode R_PUBLIC_API openFile(char* access);

private:
    File            m_file;
    UPtr            m_cursor;
    std::string     m_filepath;
};


// Simple reader wrapper, only for reading.
class ArchiveReader : public Archive
{
public:
    ArchiveReader(const std::string& filePath)
        : Archive(filePath, "r") { }
};


// Simple writer wrapper, only for writing.
class ArchiveWriter : public Archive
{
public:
    ArchiveWriter(const std::string& filePath)
        : Archive(filePath, "w") { }
};
} // Recluse