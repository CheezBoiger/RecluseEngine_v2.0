//
#pragma once 

#include "Recluse/Graphics/GraphicsContext.hpp"

namespace Recluse {


class D3D12Context : public GraphicsContext {
public:

private:
    void queryGraphicsAdapters() override { }
    void freeGraphicsAdapters() override { }
    ErrType onInitialize(const ApplicationInfo& appInfo, EnableLayerFlags flags) override { return REC_RESULT_NOT_IMPLEMENTED; }
    void onDestroy() override { }
};
} // Recluse