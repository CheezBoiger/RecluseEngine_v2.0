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
struct TextureViewID {
    Hash64 resourceCrC;
    ResourceViewType type;
    ResourceFormat format;
    ResourceViewDimension dimension;
};

class R_PUBLIC_API TextureResource {
public:
    TextureResource() { }
    virtual ~TextureResource() { }

    Hash64 getCrC() const { return m_crc; }
    void genCrC(void* pUnique, U64 sz);

private:
    Hash64 m_crc;
};

class R_PUBLIC_API Texture2D : public TextureResource {
public:
    Texture2D() 
        : m_resource(nullptr) { }

    ErrType initialize(Renderer* pRenderer, ResourceFormat format, U32 width, U32 height, U32 arrayLevel, U32 mips);
    void destroy(Renderer* pRenderer);

    // Load texture data to this resource handle.
    void load(Renderer* pRenderer, void* pData, U64 szBytes);

    GraphicsResource* getResource() { return m_resource; }

    TextureView* getTextureView(const TextureViewID& id);

private:
    GraphicsResource* m_resource;
};


class R_PUBLIC_API TextureView : public TextureResource {
public:
    TextureView()
        : m_texture(nullptr)
        , m_view(nullptr) { }

    // initializes and stores the texture view into the texture database.
    ErrType initialize(Renderer* pRenderer, Texture2D* pTexture, ResourceViewDesc& desc);

    // destroys this texture view, along with the lookup from the texture database.
    ErrType destroy(Renderer* pRenderer);

    Texture2D* getTexture() const { return m_texture; }
    GraphicsResourceView* getView() const { return m_view; }

private:
    GraphicsResourceView* m_view;
    Texture2D* m_texture;
    ResourceViewDimension m_viewDim;
    ResourceViewType m_viewType;
    ResourceFormat m_viewFormat;
};


enum SurfaceType {
    SURFACE_OPAQUE              = (1 << 0),
    SURFACE_TRANSPARENT         = (1 << 1),
    SURFACE_TRANSPARENT_CUTOUT  = (1 << 2),
    SURFACE_SUBSURFACE          = (1 << 3),
    SURFACE_FLAT                = (1 << 4),
    SURFACE_PARTICLE            = (1 << 5),
    SURFACE_SHADOWS             = (1 << 6),
    SURFACE_SELF_SHADOW         = (1 << 7)
};

typedef U32 SurfaceTypeFlags;

enum MaterialType {
    MATERIAL_TYPE_PBR_ROUGH_METAL,
    MATERIAL_TYPE_PBR_GLOSS_SPEC,
    MATERIAL_TYPE_PHONG_DIFFUSE_SPEC,
    MATERIAL_TYPE_FLAT,
    MATERIAL_TYPE_OTHER
};

#define R_MAT_ALBEDO        "Albedo"
#define R_MAT_NORMAL        "Normal"
#define R_MAT_AO            "AmbientOcclusion"
#define R_MAT_ROUGHMETAL    "RoughMetal"
#define R_MAT_GLOSSSPEC     "GlossSpec"
#define R_MAT_HEIGHT        "HeightMap"
#define R_MAT_LIGHTMAP      "LightMap"
#define R_MAT_EMISSIVE      "Emissive"

class Material {
public:
    virtual ~Material() { }

    R_PUBLIC_API Material(const std::string& matName, MaterialType type) 
        : m_matType(type)
        , m_flags(0)
        , m_matName(matName) { }

    R_PUBLIC_API MaterialType getMatType() const { return m_matType; }

    R_PUBLIC_API B32 addTex(Texture2D* pTexture, const std::string& attrib) {
        m_matMap[attrib] = pTexture;
        return true;
    }

    R_PUBLIC_API B32 hasTex(const std::string& attrib) {
        return m_matMap.find(attrib) != m_matMap.end();
    }

    R_PUBLIC_API Texture2D* getTex(const std::string& attrib) {
        return m_matMap[attrib];
    }

    R_PUBLIC_API B32 removeTex(const std::string& attrib) {
        if (hasTex(attrib)) {
            m_matMap.erase(attrib);
        }
        return false;    
    }

    R_PUBLIC_API void setSurfaceType(SurfaceTypeFlags flags) { m_flags = flags; }

    R_PUBLIC_API SurfaceTypeFlags getSurfaceTypeFlags() { return m_flags; }

    const std::string& getName() const { return m_matName; }

protected:
    SurfaceTypeFlags m_flags;
    MaterialType m_matType;
    std::unordered_map<std::string, Texture2D*> m_matMap;
    std::string m_matName;
};


// Need to call these in order to properly create textures.
R_PUBLIC_API void initializeTextureLUT();
R_PUBLIC_API void cleanupTextureLUT();

R_PUBLIC_API TextureView* lookupTextureView(const TextureViewID& id);
R_PUBLIC_API ErrType addTextureView(const TextureViewID& id);
R_PUBLIC_API ErrType removeTextureView(const TextureViewID& id);
} // Engine
} // Recluse