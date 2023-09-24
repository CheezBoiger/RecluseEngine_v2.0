//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Format.hpp"

#include <vector>

namespace Recluse {
namespace Pipeline {


enum CompressionFormat
{
    CompressionFormat_BC1,
    CompressionFormat_BC2,
    CompressionFormat_BC3,
    CompressionFormat_BC4,
    CompressionFormat_BC5,
    CompressionFormat_BC7
};


enum TextureExt
{
    TextureExt_PNG,
    TextureExt_JPG,
    TextureExt_DDS,
    TextureExt_TGA
};


// This structure describes the subresource within a texture. Should be used to read from
// a texture, for each of its subresource. 
class R_PUBLIC_API Subresource
{
public:
    Subresource(U32 width = 1, U32 height = 1, UPtr offsetAddress = 0u)
        : m_width(width)
        , m_height(height)
        , m_offsetAddress(offsetAddress)
    { }
    // Get the width of the subresource.
    U32                 getWidth() const { return m_width; }

    // Get the height of the subresource.
    U32                 getHeight() const { return m_height; }

    // Offset address of the subresource, in bytes.
    UPtr                getOffsetAddress() const { return m_offsetAddress; }

private:
    UPtr                m_offsetAddress;
    U32                 m_width;
    U32                 m_height;
};

R_PUBLIC_API U32 obtainFormatBytes(U32 resourceFormat);
// Texture which is stored in host side memory, from raw perspective.
// This structure does not determine the layout of images from certain formats (PNG, JPG, etc...)
// For that, the programmer will need to figure out how to read those formats, and store the information into Texture structure, in order to use Pipeline features.
// 
// This resource also doesn't pertain to how a device (ex. GPU) will allocate based on texture layout (linear, or optimal) so be sure to 
// call on the device api to figure out possible layout information of textures.
class R_PUBLIC_API Texture
{
public:
    Texture(const std::string& name, U32 initialWidth, U32 initialHeight, U32 arrayLayers, U32 mipLevels, ResourceFormat pixelFormat);
    ~Texture();

    // Get the array layers of the texture.
    U32                 getArrayLayers() const { return m_arrayLayers; }

    // Get the number of mips in the texture.
    U32                 getMipCount() const { return m_mipCount; }

    // Get the row pitch of the image.
    U32                 getRowPitch() const { return m_rowPitch; }

    // Check if the texture is valid.
    Bool                isValid() const { return (m_baseAddressPtr != nullptr); }

    // Get the name/path of the texture.
    const std::string&  getName() const { return m_name; }

    // Get the format of the texture.
    ResourceFormat      getPixelFormat() const { return m_pixelFormat; }
    
    // Get a subresource.
    Subresource&        getSubresource(U32 arrayLayer, U32 mip) 
    {
        return m_subresources[arrayLayer][mip]; 
    }

    // Get the base address of the image data.
    UPtr                getBaseAddress() const { return reinterpret_cast<UPtr>(m_baseAddressPtr); }
    
    // Get the total size of texture in bytes.
    U32                 getTotalSizeBytes() const { return m_totalSizeBytes; }

private:
    void*                                   m_baseAddressPtr;
    // ArrayLayers * MipLevels
    std::vector<std::vector<Subresource>>   m_subresources;
    U32                                     m_arrayLayers;
    U32                                     m_mipCount;
    // rowPitch is the scanline size, or the actual size of the width, that this image was stored physically on disk.
    // Ideally you'll want to navigate each scanline of each subresource within the image, with the rowPitch.
    U32                                     m_rowPitch;
    std::string                             m_name;
    ResourceFormat                          m_pixelFormat;
    U32                                     m_totalSizeBytes;
};


enum TextureLoadFlag
{   
    TextureLoad_None            = (0),
    TextureLoad_GenerateMips    = (1 << 0),
};
typedef U32 TextureLoadFlags;


//Texture compress(const Texture& texture, CompressionFormat compressionFormat);
} // Pipeline
} // Recluse