//
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Renderer/RendererResources.hpp"

#include "RecluseEngine_exports.hpp"

#include <unordered_map>

namespace Recluse {
namespace Engine {

class Renderer;
class TextureView;

// Texture Lookup, this is also used as helpers in texture handlers.
struct TextureViewID 
{
    Hash64                  resourceCrC;
    ResourceViewType        type;
    ResourceFormat          format;
    ResourceViewDimension   dimension;
};

class RecluseEngine_PUBLIC_API TextureResource : public RendererResource
{
public:
    TextureResource() { }
    virtual ~TextureResource() { }

    Hash64 getCrC() const { return m_crc; }
    void genCrC(void* pUnique, U64 sz);

    // Evicts the resource, or stores to disk, instead of remaining in main memory.
    ResultCode evict();

    // If the resource if evicted.
    Bool isEvicted();
    
    // Streams the resource back into main memory, from disk.
    ResultCode makeResident();

private:
    Hash64 m_crc;
};

class RecluseEngine_PUBLIC_API Texture2D : public TextureResource 
{
public:
    Texture2D() 
        : m_resource(nullptr) { }

    ResultCode initialize(GraphicsDevice* pDevice, ResourceFormat format, U32 width, U32 height, U32 arrayLevel, U32 mips, const char* debugName = nullptr);
    void destroy(GraphicsDevice* pDevice);

    // Load texture data to this resource handle.
    void load(GraphicsDevice* pDevice, void* pData, U64 szBytes);

    GraphicsResource* getResource() { return m_resource; }

    TextureView* getTextureView(const TextureViewID& id);

private:
    GraphicsResource* m_resource;
};


class RecluseEngine_PUBLIC_API TextureView : public TextureResource 
{
public:
    TextureView()
        : m_texture(nullptr)
        , m_view(nullptr) { }

    // initializes and stores the texture view into the texture database.
    ResultCode              initialize(GraphicsDevice* pDevice, Texture2D* pTexture, ResourceViewDescription& desc);

    // destroys this texture view, along with the lookup from the texture database.
    ResultCode              destroy(GraphicsDevice* pDevice);

    Texture2D*              getTexture() const { return m_texture; }
    GraphicsResourceView*   getView() const { return m_view; }

private:
    GraphicsResourceView*   m_view;
    Texture2D*              m_texture;
    ResourceViewDimension   m_viewDim;
    ResourceViewType        m_viewType;
    ResourceFormat          m_viewFormat;
};


// Material shader is the system that specifies the shader that will consume the material.
class MaterialShader 
{
public:

private:
    
};


// Engine material. This usually holds onto material assets and resources for the renderer.
class Material 
{
public:
    enum DataType 
    {
        DataType_Unknown,

        DataType_Texture1d,
        DataType_Texture1dArray,

        DataType_Texture2d,
        DataType_Texture2dArray,

        DataType_Texture3d,
        DataType_Texture3dArray,

        DataType_TextureCube,
        DataType_TextureCubeArray,

        DataType_Float,
        DataType_Float2,
        DataType_Float3,
        DataType_Float4,

        DataType_UInt,
        DataType_UInt2,
        DataType_UInt3,
        DataType_UInt4,

        DataType_Int,
        DataType_Int2,
        DataType_Int3,
        DataType_Int4,

        DataType_Double,
        DataType_Double2,
        DataType_Double3,
        DataType_Double4
    };

    virtual ~Material() { }

    RecluseEngine_PUBLIC_API Material(const std::string& matName)//, MaterialType type) 
        : /* m_matType(type)
        , m_flags(0)
        , */m_matName(matName) { }

    //R_PUBLIC_API MaterialType getMatType() const { return m_matType; }

    // Adds a texture to this material.
    RecluseEngine_PUBLIC_API Material& updateTexture(Texture2D* pTexture, const std::string& attrib) 
    {
        m_resourceMap[recluseHashFast(attrib.data(), attrib.size())] = pTexture;
        return (*this);
    }

    RecluseEngine_PUBLIC_API Material& updateResource(RendererResource* pResource, const std::string& attrib)
    {
        m_resourceMap[recluseHashFast(attrib.data(), attrib.size())] = pResource;
        return (*this);
    }

    RecluseEngine_PUBLIC_API RendererResource* getResource(const std::string& attrib);

    // Checks if this material has an available resource/texture.
    // Returns true if has resource/texture, false otherwise.
    RecluseEngine_PUBLIC_API B32 hasResource(const std::string& attrib) const
    {
        return m_resourceMap.find(recluseHashFast(attrib.data(), attrib.size())) != m_resourceMap.end();
    }

    RecluseEngine_PUBLIC_API Texture2D* getTexture(const std::string& attrib) 
    {
        return (Texture2D*)m_resourceMap[recluseHashFast(attrib.data(), attrib.size())];
    }

    // Removes a texture with the attribute. This will only nullify the texture slot,
    // in order to clean up, you must call restructure().
    RecluseEngine_PUBLIC_API B32 removeTexture(const std::string& attrib) 
    {
        if (hasResource(attrib)) 
        {
            m_resourceMap.erase(recluseHashFast(attrib.data(), attrib.size()));
            return true;
        }

        return false;    
    }

    //R_PUBLIC_API void               setSurfaceType(SurfaceTypeFlags flags) { m_flags = flags; }

    //R_PUBLIC_API SurfaceTypeFlags   getSurfaceTypeFlags() { return m_flags; }

    const std::string&              getName() const { return m_matName; }

    // Declares attributes within the material. Any already declared attribs will be ignored.
    RecluseEngine_PUBLIC_API Material& declare(const std::string& attrib)
    {
        Hash64 hash = recluseHashFast(attrib.data(), attrib.size());
        auto iter = m_resourceMap.find(hash);
        if (iter == m_resourceMap.end())
        {
            m_resourceMap[hash] = nullptr;
        }
        return (*this);
    }

    // Clear the whole material.
    RecluseEngine_PUBLIC_API void               clear() { m_resourceMap.clear(); } 

protected:

    //SurfaceTypeFlags                        m_flags;
    //MaterialType                            m_matType;
    std::unordered_map<Hash64, RendererResource*>      m_resourceMap;
    std::string                                        m_matName;
};


// Need to call these in order to properly create textures.
RecluseEngine_PUBLIC_API void           initializeTextureLUT();
RecluseEngine_PUBLIC_API void           cleanupTextureLUT();

RecluseEngine_PUBLIC_API TextureView*   lookupTextureView(const TextureViewID& id);
RecluseEngine_PUBLIC_API ResultCode        addTextureView(const TextureViewID& id);
RecluseEngine_PUBLIC_API ResultCode        removeTextureView(const TextureViewID& id);
} // Engine
} // Recluse