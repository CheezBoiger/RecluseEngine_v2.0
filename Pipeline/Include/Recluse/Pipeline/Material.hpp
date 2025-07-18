//
#pragma once
#include "Recluse/Types.hpp"

#include "ReclusePipeline_exports.hpp"

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

#define R_MATERIAL_DIFFUSE       "Diffuse"
#define R_MATERIAL_SPECULAR      "Specular"
#define R_MATERIAL_ALBEDO        "Albedo"
#define R_MATERIAL_NORMAL        "Normal"
#define R_MATERIAL_AO            "AmbientOcclusion"
#define R_MATERIAL_ROUGHNESS     "Roughness"
#define R_MATERIAL_ROUGHNESSMAP  "RoughnessMap"
#define R_MATERIAL_ROUGHMETAL    "RoughMetal"
#define R_MATERIAL_GLOSSSPEC     "GlossSpec"
#define R_MATERIAL_HEIGHT        "HeightMap"
#define R_MATERIAL_LIGHTMAP      "LightMap"
#define R_MATERIAL_EMISSIVE      "Emissive"

#define R_MATERIAL_TYPE_         "Type"


class Material 
{
public:
    virtual ~Material() { }

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
    typedef u8 data_type;

    ReclusePipeline_PUBLIC_API Material(const std::string& matName = "", const std::string& materialTypeName = "") 
        : m_matType(materialTypeName)
        , m_flags(0)
        , m_matName(matName) { }

    ReclusePipeline_PUBLIC_API std::string getMatType() const { return m_matType; }

    ReclusePipeline_PUBLIC_API B32 addTexture(Texture* pTexture, const std::string& attrib) 
    {
        U32 index = m_textures.size();
        m_textures.push_back(pTexture);
        m_matMap[attrib] = index;
        return true;
    }

    ReclusePipeline_PUBLIC_API B32 hasTexture(const std::string& attrib) const
    {
        return m_matMap.find(attrib) != m_matMap.end();
    }

    ReclusePipeline_PUBLIC_API Texture* getTexture(const std::string& attrib) 
    {
        return m_textures[m_matMap[attrib]];
    }

    // Removes a texture with the attribute. This will only nullify the texture slot,
    // in order to clean up, you must call restructure().
    ReclusePipeline_PUBLIC_API B32 removeTexture(const std::string& attrib) 
    {
        if (hasTexture(attrib)) 
        {
            m_textures[m_matMap[attrib]] = nullptr;
            m_matMap.erase(attrib);
        }

        return false;    
    }

    ReclusePipeline_PUBLIC_API void               setSurfaceType(SurfaceTypeFlags flags) { m_flags = flags; }

    ReclusePipeline_PUBLIC_API SurfaceTypeFlags   getSurfaceTypeFlags() { return m_flags; }

    const std::string&              getName() const { return m_matName; }

    Texture* const*                 getResources() { return m_textures.data(); }

    // reorganizes the material structure. Any empty slots, will now be cleaned off and sorted.
    // Be sure to update any dirty references that may be referencing a given texture.
    ReclusePipeline_PUBLIC_API void               restructure();

    // Clear the whole material.
    ReclusePipeline_PUBLIC_API void               clear();

protected:

    SurfaceTypeFlags                        m_flags;
    std::string                             m_matType;
    std::unordered_map<std::string, U32>    m_matMap;
    std::vector<Texture*>                   m_textures;
    std::string                             m_matName;
};
} // Pipeline
} // Recluse