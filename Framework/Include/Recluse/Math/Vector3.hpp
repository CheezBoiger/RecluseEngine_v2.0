//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector2.hpp"


namespace Recluse {


struct Float3 {
    union {
        struct { F32 x, y, z; };
        struct { F32 r, g, b; };
        struct { F32 u, v, w; };
    };
};


struct Int3 {
    union {
        struct { I32 x, y, z; };
        struct { I32 r, g, b; };
        struct { I32 u, v, w; };
    };
};


struct UInt3 {
    union {
        struct { U32 x, y, z; };
        struct { U32 r, g, b; };
        struct { U32 u, v, w; };
    };
};


struct Short3 {
    union {
        struct { I16 x, y, z; };
        struct { I16 r, g, b; };
        struct { I16 u, v, w; };
    };
};


struct UShort3 {
    union {
        struct { U16 x, y, z; };
        struct { U16 r, g, b; };
        struct { U16 u, v, w; };
    };
};
} // Reclue