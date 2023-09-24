//
#include "Recluse/Pipeline/Texture.hpp"

namespace Recluse {
namespace Pipeline {


U32 obtainFormatBytes(U32 pixelFormat)
{
    switch (pixelFormat)
    {
        case ResourceFormat_B8G8R8A8_Srgb:
        case ResourceFormat_R8G8B8A8_Unorm:
        case ResourceFormat_R32_Uint:
        case ResourceFormat_R32_Int:
        case ResourceFormat_R32_Float:
        case ResourceFormat_R16G16_Float:
        case ResourceFormat_R11G11B10_Float:
            return 4u;

        case ResourceFormat_R16_Float:
        case ResourceFormat_R16_Uint:
            return 2u;

        case ResourceFormat_R32G32_Float:
        case ResourceFormat_R32G32_Uint:
        case ResourceFormat_R16G16B16A16_Float:
        case ResourceFormat_BC1_Unorm:
            return 8u;

        case ResourceFormat_R8_Uint:
            return 1u;

        case ResourceFormat_R32G32B32A32_Float:
        case ResourceFormat_R32G32B32A32_Uint:
            return 16u;

        case ResourceFormat_R32G32B32_Float:
            return 12u;
    }

    return 1u;
}


Texture::Texture(const std::string& name, U32 initialWidth, U32 initialHeight, U32 arrayLayers, U32 mipLevels, ResourceFormat pixelFormat)
    : m_pixelFormat(pixelFormat)
    , m_arrayLayers(arrayLayers)
    , m_mipCount(mipLevels)
    , m_name(name)
    , m_rowPitch(0u)
    , m_baseAddressPtr(nullptr)
{
    U32 sizeBytes   = 0u;
    U32 mipWidth    = initialWidth;
    U32 mipHeight   = initialHeight;
    U32 pixelBytes  = obtainFormatBytes(pixelFormat);
    // Ideally it might end up needing to be used for alignment.
    m_rowPitch = initialWidth * pixelBytes;
    m_subresources.resize(arrayLayers);
    for (U32 layer = 0; layer < arrayLayers; ++layer)
    {
        m_subresources[layer].resize(mipLevels);
        for (U32 mip = 0; mip < mipLevels; ++mip)
        {
            m_subresources[layer][mip] = Subresource(mipWidth, mipHeight, sizeBytes);
            sizeBytes += mipHeight * m_rowPitch;
            mipWidth /= 2;
            mipHeight /= 2;
        }
    }

    if (sizeBytes > 0)
    {
        m_baseAddressPtr = malloc(sizeof(U8) * sizeBytes);
    }
    m_totalSizeBytes = sizeBytes;
}


Texture::~Texture()
{
    if (m_baseAddressPtr)
    {
        free(m_baseAddressPtr);
        m_baseAddressPtr = nullptr;
    }
}
} // Pipeline
} // Recluse