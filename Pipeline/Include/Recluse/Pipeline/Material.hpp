//
#pragma once
#include "Recluse/Types.hpp"

#include <unordered_map>
#include <vector>

namespace Recluse {
namespace Pipeline {

class Texture;

enum SurfaceType 
{
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

enum MaterialType 
{
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


class Material 
{
public:
    virtual ~Material() { }

    R_PUBLIC_API Material(const std::string& matName, MaterialType type) 
        : m_matType(type)
        , m_flags(0)
        , m_matName(matName) { }

    R_PUBLIC_API MaterialType getMatType() const { return m_matType; }

    R_PUBLIC_API B32 addTexture(Texture* pTexture, const std::string& attrib) 
    {
        U32 index = m_textures.size();
        m_textures.push_back(pTexture);
        m_matMap[attrib] = index;
        return true;
    }

    R_PUBLIC_API B32 hasTexture(const std::string& attrib) const
    {
        return m_matMap.find(attrib) != m_matMap.end();
    }

    R_PUBLIC_API Texture* getTexture(const std::string& attrib) 
    {
        return m_textures[m_matMap[attrib]];
    }

    // Removes a texture with the attribute. This will only nullify the texture slot,
    // in order to clean up, you must call restructure().
    R_PUBLIC_API B32 removeTexture(const std::string& attrib) 
    {
        if (hasTexture(attrib)) 
        {
            m_textures[m_matMap[attrib]] = nullptr;
            m_matMap.erase(attrib);
        }

        return false;    
    }

    R_PUBLIC_API void               setSurfaceType(SurfaceTypeFlags flags) { m_flags = flags; }

    R_PUBLIC_API SurfaceTypeFlags   getSurfaceTypeFlags() { return m_flags; }

    const std::string&              getName() const { return m_matName; }

    Texture* const*                 getResources() { return m_textures.data(); }

    // reorganizes the material structure. Any empty slots, will now be cleaned off and sorted.
    // Be sure to update any dirty references that may be referencing a given texture.
    R_PUBLIC_API void               restructure();

    // Clear the whole material.
    R_PUBLIC_API void               clear();

protected:

    SurfaceTypeFlags                        m_flags;
    MaterialType                            m_matType;
    std::unordered_map<std::string, U32>    m_matMap;
    std::vector<Texture*>                   m_textures;
    std::string                             m_matName;
};
} // Pipeline
} // Recluse