//
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType Archive::open(char* access)
{
    ErrType result = REC_RESULT_OK;

    result = m_file.open(m_filepath, access);
    
    if (m_file.isOpen()) {
    
        m_cursor = 0000;
    
    }

    m_filepath = m_filepath;

    return result;
}


ErrType Archive::close()
{
    if (m_file.isOpen()) {
    
        m_file.close();

    }
    
    return REC_RESULT_OK;
}


ErrType Archive::write(void* ptr, U64 sz)
{
    ErrType result = REC_RESULT_OK;

    if (m_file.isOpen()) {
        result = m_file.write(ptr, sz);
    } else {
        result = REC_RESULT_NOT_FOUND;
    }
    
    return result;
}


ErrType Archive::read(void* ptr, U64 sz)
{
    ErrType result = REC_RESULT_OK;
    if (m_file.isOpen()) {
        result = m_file.read(ptr, sz);
    } else {
        result = REC_RESULT_NOT_FOUND;
    }

    return result;
}
} // Recluse