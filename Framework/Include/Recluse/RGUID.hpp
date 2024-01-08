//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"

#include <xhash>

namespace Recluse {

struct RGUID 
{
    static const U64 kInvalidValue = ~0ULL;

    RGUID(U64 minor = kInvalidValue, U64 major = kInvalidValue)
        : version{minor, major} { }

    union 
    {
        struct 
        {
            U64 major;
            U64 minor;
        } version;

        struct 
        {
            U32 hash0;
            U32 hash1;
            U32 hash2;
            U32 hash3;
        } ss;
    };


    Bool operator==(const RGUID& rh) const
    {
        return (version.major == rh.version.major) &&
                (version.minor == rh.version.minor);
    }

    Bool operator!=(const RGUID& rh) const
    {
        return !((*this) == rh);
    }

    Bool isValid() const
    {
        return (version.major != kInvalidValue) && (version.minor != kInvalidValue);
    }

    // Hash representation function for RGUID. Use this for STD data structures that require
    // hashing for storage.
    struct Hash
    {
        bool operator()(const RGUID& rguid) const
        {
            return std::hash<U64>()(rguid.version.major)
                    ^ std::hash<U64>()(rguid.version.minor);
        }
    };
};

//
R_PUBLIC_API RGUID generateRGUID(U64 seed = 0);
} // Recluse