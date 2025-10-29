//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"

#include "RecluseFramework_exports.hpp"

namespace Recluse {

typedef void* FunctionHandle;

class R_OS_SPECIFIC RecluseFramework_PUBLIC_API DllLoader 
{
public:
    DllLoader(const std::string& dllName = "");
    ~DllLoader();

    Bool isLoaded();

    Bool load(const std::string& dllName);
    Bool unload();

    FunctionHandle procAddress(const std::string& name);

private:
    std::string     name;
    void*           library;
};
} // Recluse