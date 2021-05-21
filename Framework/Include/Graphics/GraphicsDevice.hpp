//
#pragma once

#include "Core/Types.hpp"

namespace Recluse {


class GraphicsAdapter;


class GraphicsDevice {
public:
    virtual ErrType initialize(const GraphicsAdapter& iadapter) { return 0; }
    virtual void destroy() { }

    virtual ErrType createResource() { return 0; }
    virtual ErrType createCommandList() { return 0; }
    virtual ErrType createCommandQueue() { return 0; }
private:
};
} // Recluse