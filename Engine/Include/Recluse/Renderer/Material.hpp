//
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

#include "Recluse/Serialization/Hasher.hpp"

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

class R_PUBLIC_API TextureResource 
{
public:
    TextureResource() { }
    virtual ~TextureResource() { }

    Hash64 getCrC() const { return m_crc; }
    void genCrC(void* pUnique, U64 sz);

private:
    Hash64 m_crc;
};

class R_PUBLIC_API Texture2D : public TextureResource 
{
public:
    Texture2D() 
        : m_resource(nullptr) { }

    ResultCode initialize(Renderer* pRenderer, ResourceFormat format, U32 width, U32 height, U32 arrayLevel, U32 mips);
    void destroy(Renderer* pRenderer);

    // Load texture data to this resource handle.
    void load(Renderer* pRenderer, void* pData, U64 szBytes);

    GraphicsResource* getResource() { return m_resource; }

    TextureView* getTextureView(const TextureViewID& id);

private:
    GraphicsResource* m_resource;
};


class R_PUBLIC_API TextureView : public TextureResource 
{
public:
    TextureView()
        : m_texture(nullptr)
        , m_view(nullptr) { }

    // initializes and stores the texture view into the texture database.
    ResultCode initialize(Renderer* pRenderer, Texture2D* pTexture, ResourceViewDescription& desc);

    // destroys this texture view, along with the lookup from the texture database.
    ResultCode destroy(Renderer* pRenderer);

    Texture2D* getTexture() const { return m_texture; }
    GraphicsResourceView* getView() const { return m_view; }

private:
    GraphicsResourceView* m_view;
    Texture2D* m_texture;
    ResourceViewDimension m_viewDim;
    ResourceViewType m_viewType;
    ResourceFormat m_viewFormat;
};


// Engine material.
class Material 
{
public:
    virtual ~Material() { }

    R_PUBLIC_API Material(const std::string& matName)//, MaterialType type) 
        : /* m_matType(type)
        , m_flags(0)
        , */m_matName(matName) { }

    //R_PUBLIC_API MaterialType getMatType() const { return m_matType; }

    R_PUBLIC_API B32 addTex(Texture2D* pTexture, const std::string& attrib) 
    {
        U32 index = m_textures.size();
        m_textures.push_back(pTexture);
        m_matMap[attrib] = index;
        return true;
    }

    R_PUBLIC_API B32 hasTex(const std::string& attrib) const
    {
        return m_matMap.find(attrib) != m_matMap.end();
    }

    R_PUBLIC_API Texture2D* getTex(const std::string& attrib) 
    {
        return m_textures[m_matMap[attrib]];
    }

    // Removes a texture with the attribute. This will only nullify the texture slot,
    // in order to clean up, you must call restructure().
    R_PUBLIC_API B32 removeTex(const std::string& attrib) 
    {
        if (hasTex(attrib)) 
        {
            m_textures[m_matMap[attrib]] = nullptr;
            m_matMap.erase(attrib);
        }

        return false;    
    }

    //R_PUBLIC_API void               setSurfaceType(SurfaceTypeFlags flags) { m_flags = flags; }

    //R_PUBLIC_API SurfaceTypeFlags   getSurfaceTypeFlags() { return m_flags; }

    const std::string&              getName() const { return m_matName; }

    Texture2D* const*               getResources() { return m_textures.data(); }

    // reorganizes the material structure. Any empty slots, will now be cleaned off and sorted.
    // Be sure to update any dirty references that may be referencing a given texture.
    R_PUBLIC_API void               restructure();

    // Clear the whole material.
    R_PUBLIC_API void               clear();

protected:

    //SurfaceTypeFlags                        m_flags;
    //MaterialType                            m_matType;
    std::unordered_map<std::string, U32>    m_matMap;
    std::vector<Texture2D*>                 m_textures;
    std::string                             m_matName;
};


// Need to call these in order to properly create textures.
R_PUBLIC_API void           initializeTextureLUT();
R_PUBLIC_API void           cleanupTextureLUT();

R_PUBLIC_API TextureView*   lookupTextureView(const TextureViewID& id);
R_PUBLIC_API ResultCode        addTextureView(const TextureViewID& id);
R_PUBLIC_API ResultCode        removeTextureView(const TextureViewID& id);
} // Engine
} // Recluse