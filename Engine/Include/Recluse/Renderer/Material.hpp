//
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include <unordered_map>

namespace Recluse {
namespace Engine {

class Renderer;

class Texture2D {
public:

    void initialize(Renderer* pRenderer, ResourceFormat format, U32 width, U32 height, U32 arrayLevel, U32 mips);
    void destroy(Renderer* pRenderer);

    // Load texture data to this resource handle.
    void load(Renderer* pRenderer, void* pData, U64 szBytes);

    GraphicsResource* getResource() { return m_resource; }
    GraphicsResourceView* getView() { return m_pView; }

    Hash64 getCrC() const { return m_crc; }

private:
    Hash64 m_crc;
    GraphicsResource* m_resource;
    GraphicsResourceView* m_pView;
};


enum SurfaceType {
    SURFACE_TYPE_OPAQUE = (1 << 0),
    SURFACE_TYPE_TRANSPARENT = (1 << 1),
    SURFACE_TYPE_TRANSPARENT_CUTOUT = (1 << 2),
    SURFACE_TYPE_SUBSURFACE = (1 << 3),
    SURFACE_TYPE_FLAT = (1 << 4),
    SURFACE_TYPE_PARTICLE = (1 << 5)
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

struct TextureBind {
    Texture2D* pTexture;
    std::string attrib;
};


class Material {
public:
    virtual ~Material() { }

    Material(MaterialType type) 
        : m_matType(type)
        , m_flags(0) { }

    MaterialType getMatType() const { return m_matType; }

    void initalize(TextureBind* pBinds, U32 bindCount) {
        for (U32 i = 0; i < bindCount; ++i) {
            TextureBind& bind = pBinds[i];
            m_matMap[bind.attrib] = bind.pTexture;
        }
    }

    Texture2D* getTex(const std::string& attrib) {
        return m_matMap[attrib];
    }

    void setSurfaceType(SurfaceTypeFlags flags) { m_flags = flags; }

    SurfaceTypeFlags getSurfaceTypeFlags() { return m_flags; }

protected:
    SurfaceTypeFlags m_flags;
    MaterialType m_matType;
    std::unordered_map<std::string, Texture2D*> m_matMap;
};
} // Engine
} // Recluse