//
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include <unordered_map>

namespace Recluse {
namespace Engine {

class Renderer;

class R_PUBLIC_API Texture2D {
public:
    Texture2D() 
        : m_resource(nullptr) { }

    ErrType initialize(Renderer* pRenderer, ResourceFormat format, U32 width, U32 height, U32 arrayLevel, U32 mips);
    void destroy(Renderer* pRenderer);

    // Load texture data to this resource handle.
    void load(Renderer* pRenderer, void* pData, U64 szBytes);

    GraphicsResource* getResource() { return m_resource; }

    Hash64 getCrC() const { return m_crc; }

private:
    Hash64 m_crc;
    GraphicsResource* m_resource;
};


class R_PUBLIC_API TextureView {
public:
    TextureView()
        : m_texture(nullptr)
        , m_view(nullptr) { }

    ErrType initialize(Renderer* pRenderer, Texture2D* pTexture, ResourceViewDesc& desc);
    ErrType destroy(Renderer* pRenderer);

    Texture2D* getTexture() const { return m_texture; }
    GraphicsResourceView* getView() const { return m_view; }
private:
    GraphicsResourceView* m_view;
    Texture2D* m_texture;
};


enum SurfaceType {
    SURFACE_OPAQUE = (1 << 0),
    SURFACE_TRANSPARENT = (1 << 1),
    SURFACE_TRANSPARENT_CUTOUT = (1 << 2),
    SURFACE_SUBSURFACE = (1 << 3),
    SURFACE_FLAT = (1 << 4),
    SURFACE_PARTICLE = (1 << 5),
    SURFACE_SHADOWS = (1 << 6),
    SURFACE_SELF_SHADOW = (1 << 7)
};

typedef U32 SurfaceTypeFlags;

enum MaterialType {
    MATERIAL_TYPE_PBR_ROUGH_METAL,
    MATERIAL_TYPE_PBR_GLOSS_SPEC,
    MATERIAL_TYPE_OTHER
};

#define R_MAT_ALBEDO "Albedo"
#define R_MAT_NORMAL "Normal"
#define R_MAT_AO "Ao"
#define R_MAT_ROUGHMETAL "RoughMetal"
#define R_MAT_GLOSSSPEC "GlossSpec"
#define R_MAT_HEIGHT "HeightMap"

class Material {
public:
    virtual ~Material() { }

    R_PUBLIC_API Material(MaterialType type) 
        : m_matType(type)
        , m_flags(0) { }

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

protected:
    SurfaceTypeFlags m_flags;
    MaterialType m_matType;
    std::unordered_map<std::string, Texture2D*> m_matMap;
};
} // Engine
} // Recluse