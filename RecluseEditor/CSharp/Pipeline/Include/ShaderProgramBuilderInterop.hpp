//
#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"

#include <vcclr.h>

#using <WindowsBase.dll>
#using <mscorlib.dll>


namespace Recluse {
namespace CSharp {
namespace Pipeline {

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Interop;

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


public ref class ShaderProgramDescription
{
public:
    //ShaderProgramDescription(ShaderLanguage Language, UInt64 ProgramId)
    //    : Language(Language)
    //    , ProgramId(ProgramId)
    //{ }

    CSharp::Pipeline::ShaderLanguage Language;
    UInt64                           ProgramId; // Given program id to associate this shader program with.
};


public ref class Raster3DShaderProgramDescription : public ShaderProgramDescription
{
public:
    String^ PS;
    String^ PSName;
};


public ref class RasterShaderProgramDescription : public Raster3DShaderProgramDescription
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
};


public ref class MeshShaderProgramDescription : public Raster3DShaderProgramDescription
{
public:
    // Amp shader.
    String^ AS;
    String^ ASName;
    // Mesh shader.
    String^ MS;
    String^ MSName;
};


public ref class ComputeShaderProgramDescription : public ShaderProgramDescription
{
public:
    String^ CS;
    String^ CSName;
};


public ref class RayTracingShaderProgramDescription : public ShaderProgramDescription
{
public:
};


public ref class ShaderProgramBuilder
{
public:
    ShaderProgramBuilder();

    bool PushDescription(ShaderProgramDescription^ Description);
    //
    bool SaveToDisk();
    bool LoadFromDisk();
    bool Build(ShaderIntermediateLanguage IntermediateLanguage);
    void Clear();
private:
    ShaderProgramDatabase* Database;
    Array^ Descriptions;
};
} // Pipeline
} // CSharp
} // Recluse