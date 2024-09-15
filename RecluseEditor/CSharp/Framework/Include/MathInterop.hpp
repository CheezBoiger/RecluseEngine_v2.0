//
#pragma once

#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector4.hpp"

#include "Recluse/Math/Matrix33.hpp"
#include "Recluse/Math/Matrix43.hpp"
#include "Recluse/Math/Vector3.hpp"

#include "Recluse/Math/Matrix22.hpp"
#include "Recluse/Math/Vector2.hpp"

#using <WindowsBase.dll>
#using <mscorlib.dll>

#pragma managed

namespace Recluse {
namespace CSharp {
namespace Math {

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Interop;


//public ref class Matrix44 
//{
//public:
//
//    Matrix44();
//    !Matrix44();
//
//
//private:
//    Recluse::Math::Matrix44* m44;
//};


//public ref class Vector4
//{
//public:
//    Vector4()
//        : v4(new Recluse::Math::Float4())
//    { }
//
//    Vector4(System::Single X)
//        : v4(new Recluse::Math::Float4(X))
//    { }
//
//    Vector4(System::Single X, System::Single Y)
//        : v4(new Recluse::Math::Float4(X, Y))
//    { }
//
//    Vector4(System::Single X, System::Single Y, System::Single Z)
//        : v4(new Recluse::Math::Float4(X, Y, Z))
//    { }
//
//    Vector4(System::Single X, System::Single Y, System::Single Z, System::Single W)
//        : v4(new Recluse::Math::Float4(X, Y, Z, W))
//    { }
//
//    !Vector4()
//    {
//        if (v4) delete v4;
//        v4 = nullptr;
//    }
//
//    System::Single X() { return v4->x; }
//    System::Single Y() { return v4->y; }
//    System::Single Z() { return v4->z; }
//    System::Single W() { return v4->w; }
//
//    Vector4^ operator+(Vector4^ A) 
//    { 
//        Recluse::Math::Float4 value = *v4 + *A->v4;
//        return gcnew Vector4(value); 
//    }
//
//    Vector4^ operator-(Vector4^ A) 
//    { 
//        Recluse::Math::Float4 value = *v4 - *A->v4;
//        return gcnew Vector4(value); 
//    }
//
//    Vector4^ operator*(Vector4^ A) 
//    { 
//        Recluse::Math::Float4 value = *v4 * *A->v4;
//        return gcnew Vector4(value); 
//    }
//
//    static Vector4^ dot(Vector4^ A, Vector4^ B);    
//private:
//    Vector4(Recluse::Math::Float4 ans)
//        : v4(new Recluse::Math::Float4())
//    { *v4 = ans; }
//    Recluse::Math::Float4* v4;
//};

} // Math
} // CSharp
} // Recluse