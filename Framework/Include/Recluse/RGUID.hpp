//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "RecluseFramework_exports.hpp"

#include <xhash>

namespace Recluse {

// Unique Identifier for all objects and resources in the engine.
struct RGUID 
{
    static const U64 kInvalidValue = ~0ULL;

    RGUID
    (
          U64 major = kInvalidValue
#ifndef R_USE_64BIT_IDENTIFIERS
        , U64 minor = kInvalidValue
#endif
    )
        : version{ major
#ifndef R_USE_64BIT_IDENTIFIERS
        , minor
#endif
        }
        { }

    union 
    {
        struct 
        {
            U64 major;
#ifndef R_USE_64BIT_IDENTIFIERS
            U64 minor;
#endif
        } version;

        struct 
        {
            U32 hash0;
            U32 hash1;
#ifndef R_USE_64BIT_IDENTIFIERS
            U32 hash2;
            U32 hash3;
#endif
        } ss;
    };


    Bool operator==(const RGUID& rh) const
    {
        return (version.major == rh.version.major) 
#ifndef R_USE_64BIT_IDENTIFIERS
            && (version.minor == rh.version.minor)
#endif
            ;
    }

    Bool operator!=(const RGUID& rh) const
    {
        return !((*this) == rh);
    }

    // Check if this rguid is valid. Returns true if this guid is valid, false otherwise.
    Bool isValid() const
    {
        return (version.major != kInvalidValue)
#ifndef R_USE_64BIT_IDENTIFIERS
            && (version.minor != kInvalidValue)
#endif
            ;
    }

    // Hash representation function for RGUID. Use this for STD data structures that require
    // hashing for storage.
    struct Hash
    {
        bool operator()(const RGUID& rguid) const
        {
            return std::hash<U64>()(rguid.version.major)
#ifndef R_USE_64BIT_IDENTIFIERS
                    ^ std::hash<U64>()(rguid.version.minor)
#endif
            ;
        }
    };

    struct Less
    {
    public:
        bool operator()(const RGUID& a, const RGUID& b) const 
        {
            return (a.version.major == b.version.major) ? 
                (a.version.minor < b.version.minor) :  (a.version.major < b.version.major);
        }
    };

    struct Greater
    {
    public:
        bool operator()(const RGUID& a, const RGUID& b) const
        {
            return (a.version.major == b.version.major) ? 
                (a.version.minor > b.version.minor) :  (a.version.major > b.version.major);
        }
    };

    struct Equal
    {
    public:
        bool operator()(const RGUID& a, const RGUID& b) const
        {
            return (a.version.major == b.version.major) && (a.version.minor == b.version.minor);
        }
    };
};

// Generates an RGUID identifier, which can be used to represent unique entities and objects.
RecluseFramework_PUBLIC_API RGUID generateRGUID(U64 seed = 0);
} // Recluse