//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

class Archive;

class Serializable 
{
public:
    virtual ~Serializable() { }

    virtual ResultCode serialize(Archive* pArchive) const = 0;
    virtual ResultCode deserialize(Archive* pArchive) = 0;

};
} // Recluse