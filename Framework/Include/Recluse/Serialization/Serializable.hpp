//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

class Archive;

class Serializable {
public:
    virtual ~Serializable() { }

    virtual ErrType serialize(Archive* pArchive) = 0;
    virtual ErrType deserialize(Archive* pArchive) = 0;

};
} // Recluse