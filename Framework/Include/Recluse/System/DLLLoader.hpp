//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


class R_OS_SPECIFIC R_PUBLIC_API DllLoader 
{
public:
    DllLoader(const std::string& dllName = "");
    ~DllLoader();

    Bool isLoaded();

    Bool load(const std::string& dllName);
    Bool unload();

    void* procAddress(const std::string& name);

private:
    std::string name;
    void* library;
};
} // Recluse