//
#include "Recluse/Renderer/Texture.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Engine {


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


Texture::Texture()
    : m_pixelFormat(ResourceFormat_Unknown)
    , m_arrayLayers(0u)
    , m_mipCount(0u)
    , m_baseAddressPtr(nullptr)
    , m_rowPitch(0u)
    , m_totalSizeBytes(0u)
{
}


Texture::Texture(const std::string& name, U32 initialWidth, U32 initialHeight, U32 arrayLayers, U32 mipLevels, ResourceFormat pixelFormat)
    : m_pixelFormat(pixelFormat)
    , m_arrayLayers(arrayLayers)
    , m_mipCount(mipLevels)
    , m_name(name)
    , m_rowPitch(0u)
    , m_baseAddressPtr(nullptr)
{
    R_ASSERT_FORMAT(initialWidth > 0u && initialHeight > 0u && arrayLayers > 0u && mipLevels > 0u, "Parameters need to be greater than 0!");
    U32 sizeBytes   = 0u;
    U32 pixelBytes  = obtainFormatBytes(pixelFormat);
    // Ideally rowPitch might end up needing to be used for alignment.
    m_rowPitch = initialWidth * pixelBytes;
    m_subresources.resize(arrayLayers);
    for (U32 layer = 0; layer < arrayLayers; ++layer)
    {
        U32 mipWidth    = initialWidth;
        U32 mipHeight   = initialHeight;
        m_subresources[layer].resize(mipLevels);
        for (U32 mip = 0; mip < mipLevels; ++mip)
        {
            m_subresources[layer][mip] = Subresource(mipWidth, mipHeight, sizeBytes);
            sizeBytes += mipHeight * m_rowPitch;
            mipWidth >>= 1;
            mipHeight >>= 1;
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


Texture::Texture(Texture&& texture) noexcept
{
    std::swap(m_arrayLayers, texture.m_arrayLayers);
    std::swap(m_mipCount, texture.m_mipCount);
    std::swap(m_baseAddressPtr, texture.m_baseAddressPtr);
    std::swap(m_rowPitch, texture.m_rowPitch);
    std::swap(m_pixelFormat, texture.m_pixelFormat);
    std::swap(m_subresources, texture.m_subresources);
    std::swap(m_totalSizeBytes, texture.m_totalSizeBytes);
    std::swap(m_name, texture.m_name);
}


Texture::Texture(const Texture& texture)
{
    m_subresources.resize(texture.m_subresources.size());
    for (size_t i = 0; i < m_subresources.size(); ++i)
    {
        m_subresources[i].resize(texture.m_subresources[i].size());
        std::copy(texture.m_subresources[i].begin(), texture.m_subresources[i].end(), m_subresources[i].begin());
    }

    if (m_totalSizeBytes != texture.m_totalSizeBytes)
    {
        if (m_baseAddressPtr)
        {
            free(m_baseAddressPtr);
        }
        m_baseAddressPtr = malloc(sizeof(U8) * texture.m_totalSizeBytes);
    }

    // Perform the memory copy if we have base address.
    if (m_baseAddressPtr != 0ull)
    {
        memcpy((void*)m_baseAddressPtr, (const void*)texture.m_baseAddressPtr, texture.m_totalSizeBytes);
    }

    m_totalSizeBytes = texture.m_totalSizeBytes;
    m_arrayLayers = texture.m_arrayLayers;
    m_mipCount = texture.m_mipCount;
    m_rowPitch = texture.m_rowPitch;
    m_pixelFormat = texture.m_pixelFormat;
    m_name = texture.m_name;
}


Texture& Texture::operator=(const Texture& texture)
{
    m_subresources.resize(texture.m_subresources.size());
    for (size_t i = 0; i < m_subresources.size(); ++i)
    {
        m_subresources[i].resize(texture.m_subresources[i].size());
        std::copy(texture.m_subresources[i].begin(), texture.m_subresources[i].end(), m_subresources[i].begin());
    }

    if (m_totalSizeBytes != texture.m_totalSizeBytes)
    {
        if (m_baseAddressPtr)
        {
            free(m_baseAddressPtr);
        }
        m_baseAddressPtr = malloc(sizeof(U8) * texture.m_totalSizeBytes);
    }

    // Perform the memory copy if we have base address.
    if (m_baseAddressPtr != 0ull)
    {
        memcpy((void*)m_baseAddressPtr, (const void*)texture.m_baseAddressPtr, texture.m_totalSizeBytes);
    }

    m_totalSizeBytes = texture.m_totalSizeBytes;
    m_arrayLayers = texture.m_arrayLayers;
    m_mipCount = texture.m_mipCount;
    m_rowPitch = texture.m_rowPitch;
    m_pixelFormat = texture.m_pixelFormat;
    m_name = texture.m_name;
    return (*this);
}


Texture& Texture::operator=(Texture&& texture) noexcept
{
    std::swap(m_arrayLayers, texture.m_arrayLayers);
    std::swap(m_mipCount, texture.m_mipCount);
    std::swap(m_baseAddressPtr, texture.m_baseAddressPtr);
    std::swap(m_rowPitch, texture.m_rowPitch);
    std::swap(m_pixelFormat, texture.m_pixelFormat);
    std::swap(m_subresources, texture.m_subresources);
    std::swap(m_totalSizeBytes, texture.m_totalSizeBytes);
    std::swap(m_name, texture.m_name);
    return (*this);
}
} // Engine
} // Recluse