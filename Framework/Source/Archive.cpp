//
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


ResultCode Archive::openFile(char* access)
{
    ResultCode result = RecluseResult_Ok;

    result = m_file.open(m_filepath, access);
    
    if (m_file.isOpen()) 
    {
        m_cursor = 0000;
    }

    m_filepath = m_filepath;

    return result;
}


ResultCode Archive::close()
{
    if (m_file.isOpen()) 
    {
        m_file.close();
    }
    
    return RecluseResult_Ok;
}


ResultCode Archive::write(void* ptr, U64 sz)
{
    ResultCode result = RecluseResult_Ok;

    if (m_file.isOpen()) 
    {
        result = m_file.write(ptr, sz);
    } 
    else 
    {
        result = RecluseResult_NotFound;
    }
    
    return result;
}


ResultCode Archive::read(void* ptr, U64 sz)
{
    ResultCode result = RecluseResult_Ok;
    if (m_file.isOpen()) 
    {
        result = m_file.read(ptr, sz);
    } 
    else 
    {
        result = RecluseResult_NotFound;
    }

    return result;
}
} // Recluse