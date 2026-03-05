//
#pragma once

#include "Recluse/Types.hpp"

#include "RecluseFramework_exports.hpp"

namespace Recluse {

class Archive;

class RecluseFramework_PUBLIC_API Serializable 
{
public:
    virtual ~Serializable() { }

    virtual ResultCode serialize(Archive* pArchive) const = 0;
    virtual ResultCode deserialize(Archive* pArchive) = 0;

};
} // Recluse