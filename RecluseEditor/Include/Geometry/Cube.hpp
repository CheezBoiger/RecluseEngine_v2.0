//
#pragma once
#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Vector3.hpp"

#include <array>

namespace Recluse {
namespace Editor {

// Cube Geometry.
class Cube 
{
public:
    typedef Recluse::Math::Float4 Vector4;
    typedef Recluse::Math::Float3 Vector3;

    static std::array<Vector3, 36> getPositions() 
    {
        return std::array<Vector3, 36> {
          // front
          Vector3(-1.0f, -1.0f, 1.0f),
          Vector3( 1.0f, -1.0f, 1.0f),
          Vector3( 1.0f,  1.0f, 1.0f),
          Vector3( 1.0f,  1.0f, 1.0f),
          Vector3(-1.0f,  1.0f, 1.0f),
          Vector3(-1.0f, -1.0f, 1.0f),
          // Back
          Vector3(-1.0f, -1.0f, -1.0f),
          Vector3(-1.0f,  1.0f, -1.0f),
          Vector3( 1.0f,  1.0f, -1.0f),
          Vector3( 1.0f,  1.0f, -1.0f),
          Vector3( 1.0f, -1.0f, -1.0f),
          Vector3(-1.0f, -1.0f, -1.0f),
          // up
          Vector3( 1.0f,  1.0f,  1.0f),
          Vector3( 1.0f,  1.0f, -1.0f),
          Vector3(-1.0f,  1.0f, -1.0f),
          Vector3(-1.0f,  1.0f, -1.0f),
          Vector3(-1.0f,  1.0f,  1.0f),
          Vector3( 1.0f,  1.0f,  1.0f),
          // Down
          Vector3( 1.0f, -1.0f,  1.0f),
          Vector3(-1.0f, -1.0f,  1.0f),
          Vector3(-1.0f, -1.0f, -1.0f),
          Vector3(-1.0f, -1.0f, -1.0f),
          Vector3( 1.0f, -1.0f, -1.0f),
          Vector3( 1.0f, -1.0f,  1.0f),
          // right
          Vector3( 1.0f, -1.0f,  1.0f),
          Vector3( 1.0f, -1.0f, -1.0f),
          Vector3( 1.0f,  1.0f, -1.0f),
          Vector3( 1.0f,  1.0f, -1.0f),
          Vector3( 1.0f,  1.0f,  1.0f),
          Vector3( 1.0f, -1.0f,  1.0f),
          // Left
          Vector3(-1.0f, -1.0f,  1.0f),
          Vector3(-1.0f,  1.0f,  1.0f),
          Vector3(-1.0f,  1.0f, -1.0f),
          Vector3(-1.0f,  1.0f, -1.0f),
          Vector3(-1.0f, -1.0f, -1.0f),
          Vector3(-1.0f, -1.0f,  1.0f),
        };
    }


    static const std::array<Vector3, 36> getNormals() 
    {
        return std::array<Vector3, 36> {
          // front 
          Vector3(0.0f, 0.0f, 1.0f),
          Vector3(0.0f, 0.0f, 1.0f),
          Vector3(0.0f, 0.0f, 1.0f),
          Vector3(0.0f, 0.0f, 1.0f),
          Vector3(0.0f, 0.0f, 1.0f),
          Vector3(0.0f, 0.0f, 1.0f),
          // Back
          Vector3(0.0f, 0.0f, -1.0f),
          Vector3(0.0f, 0.0f, -1.0f),
          Vector3(0.0f, 0.0f, -1.0f),
          Vector3(0.0f, 0.0f, -1.0f),
          Vector3(0.0f, 0.0f, -1.0f),
          Vector3(0.0f, 0.0f, -1.0f),
          // up
          Vector3(0.0f, 1.0f, 0.0f),
          Vector3(0.0f, 1.0f, 0.0f),
          Vector3(0.0f, 1.0f, 0.0f),
          Vector3(0.0f, 1.0f, 0.0f),
          Vector3(0.0f, 1.0f, 0.0f),
          Vector3(0.0f, 1.0f, 0.0f),
          // Down
          Vector3(0.0f, -1.0f, 0.0f),
          Vector3(0.0f, -1.0f, 0.0f),
          Vector3(0.0f, -1.0f, 0.0f),
          Vector3(0.0f, -1.0f, 0.0f),
          Vector3(0.0f, -1.0f, 0.0f),
          Vector3(0.0f, -1.0f, 0.0f),
          // right
          Vector3(1.0f, 0.0f, 0.0f),
          Vector3(1.0f, 0.0f, 0.0f),
          Vector3(1.0f, 0.0f, 0.0f),
          Vector3(1.0f, 0.0f, 0.0f),
          Vector3(1.0f, 0.0f, 0.0f),
          Vector3(1.0f, 0.0f, 0.0f),
          // Left
          Vector3(-1.0f, 0.0f, 0.0f),
          Vector3(-1.0f, 0.0f, 0.0f),
          Vector3(-1.0f, 0.0f, 0.0f),
          Vector3(-1.0f, 0.0f, 0.0f),
          Vector3(-1.0f, 0.0f, 0.0f),
          Vector3(-1.0f, 0.0f, 0.0f)
        };
    }


    static const std::array<Vector4, 36> getTexCoords() 
    {
        return std::array<Vector4, 36> {
          // front
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
          Vector4(1.0f, 0.0f, 1.0f, 0.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),

          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
          Vector4(1.0f, 0.0f, 1.0f, 0.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),

          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
          Vector4(1.0f, 0.0f, 1.0f, 0.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),

          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
          Vector4(1.0f, 0.0f, 1.0f, 0.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
          // right
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
          Vector4(1.0f, 0.0f, 1.0f, 0.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
          // Left
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
          Vector4(1.0f, 0.0f, 1.0f, 0.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(1.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 0.0f, 0.0f, 0.0f),
        };
    }


    static const std::array<Vector4, 36> getColors() 
    {
        return std::array<Vector4, 36> {
          Vector4(0.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 1.0f, 1.0f),
          Vector4(0.0f, 1.0f, 1.0f, 1.0f),

          Vector4(1.0f, 0.0f, 1.0f, 1.0f),
          Vector4(1.0f, 0.0f, 1.0f, 1.0f),
          Vector4(1.0f, 0.0f, 1.0f, 1.0f),
          Vector4(1.0f, 0.0f, 1.0f, 1.0f),
          Vector4(1.0f, 0.0f, 1.0f, 1.0f),
          Vector4(1.0f, 0.0f, 1.0f, 1.0f),

          Vector4(1.0f, 1.0f, 0.0f, 1.0f),
          Vector4(1.0f, 1.0f, 0.0f, 1.0f),
          Vector4(1.0f, 1.0f, 0.0f, 1.0f),
          Vector4(1.0f, 1.0f, 0.0f, 1.0f),
          Vector4(1.0f, 1.0f, 0.0f, 1.0f),
          Vector4(1.0f, 1.0f, 0.0f, 1.0f),

          Vector4(1.0f, 0.0f, 0.0f, 1.0f),
          Vector4(1.0f, 0.0f, 0.0f, 1.0f),
          Vector4(1.0f, 0.0f, 0.0f, 1.0f),
          Vector4(1.0f, 0.0f, 0.0f, 1.0f),
          Vector4(1.0f, 0.0f, 0.0f, 1.0f),
          Vector4(1.0f, 0.0f, 0.0f, 1.0f),

          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),
          Vector4(0.0f, 1.0f, 0.0f, 1.0f),

          Vector4(0.0f, 0.0f, 1.0f, 1.0f),
          Vector4(0.0f, 0.0f, 1.0f, 1.0f),
          Vector4(0.0f, 0.0f, 1.0f, 1.0f),
          Vector4(0.0f, 0.0f, 1.0f, 1.0f),
          Vector4(0.0f, 0.0f, 1.0f, 1.0f),
          Vector4(0.0f, 0.0f, 1.0f, 1.0f),
        };
    }

    static const std::array<Recluse::U32, 36> getIndices() 
    {
        return std::array<Recluse::U32, 36> {
          0, 1, 2,
          3, 4, 5,
          6, 7, 8,
          9, 10, 11,
          12, 13, 14,
          15, 16, 17,
          18, 19, 20,
          21, 22, 23,
          24, 25, 26,
          27, 28, 29,
          30, 31, 32,
          33, 34, 35
        };
    }
};
} // Editor
} // Recluse