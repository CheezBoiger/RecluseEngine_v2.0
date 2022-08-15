//
#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"


#if defined(RECLUSE_EDITOR_SDK_ENABLE)
#define R_SDK_DEFINE(item) item
#else
#define R_SDK_DECLARE(item)
#endif

typedef void(*VoidEditorSdkShimFun)();
typedef Recluse::I32(*IntEditorSdkShimFun)();

R_SDK_DECLARE();