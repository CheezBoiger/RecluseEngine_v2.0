//
#include "Recluse/String.hpp"
#include "Recluse/Graphics/Format.hpp"

namespace Recluse {


String::~String()
{
    if (m_cStr)
    {
        delete m_cStr;
        m_cStr = nullptr;
    }

    m_size = 0;
}


std::string toLowerCase(const std::string& str)
{
    std::string lowerStr;
    lowerStr.resize(str.size());
    for (U32 i = 0; i < lowerStr.size(); ++i)
    {
        lowerStr[i] = std::tolower(str[i]);
    }
    return lowerStr;
}


std::string toUpperCase(const std::string& str)
{
    std::string upperStr;
    upperStr.resize(str.size());
    for (U32 i = 0; i < upperStr.size(); ++i)
    {
        upperStr[i] = std::toupper(str[i]);
    }
    return upperStr;
}

#define FORMAT_STRING(ff) case (ff): return (#ff)

const char* getResourceFormatString(ResourceFormat format)
{
    switch (format)
    {
        default:
        FORMAT_STRING(ResourceFormat_Unknown);
        FORMAT_STRING(ResourceFormat_R8G8B8A8_Unorm);
        FORMAT_STRING(ResourceFormat_R16G16B16A16_Float);
        FORMAT_STRING(ResourceFormat_R11G11B10_Float);
        FORMAT_STRING(ResourceFormat_D32_Float);
        FORMAT_STRING(ResourceFormat_D24_Unorm_S8_Uint);
        FORMAT_STRING(ResourceFormat_D32_Float_S8_Uint);
        FORMAT_STRING(ResourceFormat_R16G16_Float);
        FORMAT_STRING(ResourceFormat_B8G8R8A8_Srgb);
        FORMAT_STRING(ResourceFormat_R32G32B32A32_Float);
        FORMAT_STRING(ResourceFormat_R32G32B32A32_Uint);
        FORMAT_STRING(ResourceFormat_R8_Uint);
        FORMAT_STRING(ResourceFormat_R32G32_Float);
        FORMAT_STRING(ResourceFormat_R32G32_Uint);
        FORMAT_STRING(ResourceFormat_R16_Uint);
        FORMAT_STRING(ResourceFormat_R16_Float);
        FORMAT_STRING(ResourceFormat_B8G8R8A8_Unorm);
        FORMAT_STRING(ResourceFormat_R32G32B32_Float);
    }
}
} // Recluse