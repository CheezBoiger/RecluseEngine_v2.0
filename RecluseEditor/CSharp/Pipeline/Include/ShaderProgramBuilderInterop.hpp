//
#pragma once
#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"

#include <vcclr.h>

#using <WindowsBase.dll>
#using <mscorlib.dll>
#using <RecluseCSharpGraphics.dll>
#include <vector>

#pragma managed

namespace Recluse {
namespace CSharp {
namespace Pipeline {

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Interop;
using namespace Recluse::Pipeline;

public enum class ShaderLanguage : System::Int32
{
    Glsl,
    Hlsl
};


public enum class ShaderIntermediateLanguage : System::Int32
{
    Dxil,
    Dxbc,
    Spirv
};


public enum class PipelineType : System::Int32
{
    Graphics,
    Compute,
    Raytracing
};


public enum class ShaderCompiler : System::Int32
{
    GLSLang,
    DXC
};


public ref class ShaderProgramPermutationDefinition
{
public:
    String^         Name;
    System::UInt32  Offset;
    System::UInt32  Size;
    System::UInt32  Value;
};


public ref class ShaderProgramDescription
{
public:
    ShaderProgramDescription()
        : Language(ShaderLanguage::Hlsl)
        , Strings(new std::vector<std::string>())
    { }

    ShaderProgramDescription(ShaderLanguage Language)
        : Language(Language)
        , Strings(new std::vector<std::string>())
    { }
    virtual ~ShaderProgramDescription();
    !ShaderProgramDescription();

    CSharp::Pipeline::ShaderLanguage            Language;
    array<ShaderProgramPermutationDefinition^>^ Permutations;
    virtual void StoreIn(Builder::ShaderProgramDescription& description);

protected:
    const char* MakeNativeString(String^ Str);

private:
    std::vector<std::string>* Strings;
};


public ref class RasterShaderProgramDescription : public ShaderProgramDescription
{
public:
    String^ PS;
    String^ PSName;

    virtual void StoreIn(Builder::ShaderProgramDescription& description) override;
};


public ref class VertexRasterShaderProgramDescription : public RasterShaderProgramDescription
{
public:
    String^ VS;
    String^ VSName;

    String^ GS;
    String^ GSName;
    
    String^ HS;
    String^ HSName;

    String^ DS;
    String^ DSName;

    virtual void StoreIn(Builder::ShaderProgramDescription& description) override;
};


public ref class MeshShaderProgramDescription : public RasterShaderProgramDescription
{
public:
    // Amp shader.
    String^ AS;
    String^ ASName;
    // Mesh shader.
    String^ MS;
    String^ MSName;

    virtual void StoreIn(Builder::ShaderProgramDescription& description) override;
};


public ref class ComputeShaderProgramDescription : public ShaderProgramDescription
{
public:
    String^ CS;
    String^ CSName;

    virtual void StoreIn(Builder::ShaderProgramDescription& description) override;
};


public ref class RayTracingShaderProgramDescription : public ShaderProgramDescription
{
public:
    virtual void StoreIn(Builder::ShaderProgramDescription& description) override;

    String^ RAny;
    String^ RAnyName;
    
    String^ RClosest;
    String^ RClosestName;

    String^ RIntersect;
    String^ RIntersectName;

    String^ RMiss;
    String^ RMissName;

    String^ RGen;
    String^ RGenName;
};


public ref class ShaderProgramBuilder
{
public:
    ShaderProgramBuilder(ShaderCompiler Compiler);
    ~ShaderProgramBuilder();
    !ShaderProgramBuilder();

    bool PushDescription(ShaderProgramDescription^ Description, UInt64 ShaderProgramId);
    //
    bool SaveToDisk(String^ Filename);
    bool LoadFromDisk(String^ Filename);
    bool Build(ShaderIntermediateLanguage IntermediateLanguage);
    bool LoadToRuntime(CSharp::IGraphicsDevice^ Device);
    void Clear();
private:
    ShaderProgramDatabase* Database;
    Recluse::Pipeline::Builder::ShaderProgramDescriptionInfo* DescriptionInfo;
};
} // Pipeline
} // CSharp
} // Recluse