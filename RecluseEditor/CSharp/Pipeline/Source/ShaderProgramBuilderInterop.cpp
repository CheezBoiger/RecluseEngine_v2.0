//
#include "ShaderProgramBuilderInterop.hpp"
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Messaging.hpp"

#include <msclr/marshal_cppstd.h>

namespace Recluse {
namespace CSharp {
namespace Pipeline {


using namespace Recluse::Pipeline::Builder;


ShaderIntermediateCode CSharpToNativeIntermediateLanguage(ShaderIntermediateLanguage Language)
{
    switch (Language)
    {
        case ShaderIntermediateLanguage::Dxil:
            return ShaderIntermediateCode_Dxil;
        case ShaderIntermediateLanguage::Dxbc:
            return ShaderIntermediateCode_Dxbc;
        case ShaderIntermediateLanguage::Spirv:
        default:
            return ShaderIntermediateCode_Spirv;
    }
}


Recluse::ShaderLanguage CSharpToNativeHighLanguage(ShaderLanguage Language)
{
    switch (Language)
    {
        case ShaderLanguage::Glsl:
            return ShaderLanguage_Glsl;
        case ShaderLanguage::Hlsl:
        default:
            return ShaderLanguage_Hlsl;
    }
}


R_INTERNAL Recluse::BindType CSharpToNativePipelineType(PipelineType Pipeline)
{
    switch (Pipeline)
    {
        case PipelineType::Raytracing:
            return BindType_RayTrace;
        case PipelineType::Compute:
            return BindType_Compute;
        case PipelineType::Graphics:
        default:
            return BindType_Graphics;
    }
}


ShaderProgramBuilder::ShaderProgramBuilder(ShaderCompiler Compiler)
    : Database(nullptr)
    , DescriptionInfo(nullptr)
{
    Database = new ShaderProgramDatabase();
    DescriptionInfo = new ShaderProgramDescriptionInfo();

    switch (Compiler)
    {
        case ShaderCompiler::DXC:
            GlobalCommands::setValue("ShaderBuilder.NameId", "dxc");
            break;
        case ShaderCompiler::GLSLang:
        default:
            GlobalCommands::setValue("ShaderBuilder.NameId", "glslang");
            break;
    }
}


ShaderProgramBuilder::~ShaderProgramBuilder()
{
    if (Database)
    {
        delete Database;
    }
    if (DescriptionInfo)
    {
        delete DescriptionInfo;
    }
}


ShaderProgramBuilder::!ShaderProgramBuilder()
{
    if (Database)
    {
        delete Database;
    }

    if (DescriptionInfo)
    {
        delete DescriptionInfo;
    }

    Database = nullptr;
    DescriptionInfo = nullptr;
}


bool ShaderProgramBuilder::PushDescription(ShaderProgramDescription^ Description, UInt64 ProgramId)
{
    Builder::ShaderProgramDescription description = { };
    Description->StoreIn(description);
    DescriptionInfo->descriptions.push_back(description);
    DescriptionInfo->shaderProgramIds.push_back((ShaderProgramId)ProgramId);
    return true;
}


bool ShaderProgramBuilder::SaveToDisk(String^ Filename)
{
    std::string filename;
    filename.resize(Filename->Length);
    for (U32 i = 0; i < Filename->Length; ++i)
    {
        filename[i] = Filename[i];
    }
    ArchiveWriter writer(filename);
    if (writer.isOpen())
    {
        return (Database->serialize(&writer) == RecluseResult_Ok);
    }
    return false;
}


bool ShaderProgramBuilder::LoadFromDisk(String^ Filename)
{
    std::string filename;
    filename.resize(Filename->Length);
    for (U32 i = 0; i < Filename->Length; ++i)
    {
        filename[i] = Filename[i];
    }
    ArchiveReader reader(filename);
    if (reader.isOpen())
    {
        return (Database->deserialize(&reader) == RecluseResult_Ok);
    }
    return false;
}


bool ShaderProgramBuilder::Build(ShaderIntermediateLanguage IntermediateLanguage)
{
    R_ASSERT(DescriptionInfo->descriptions.empty() == false);
    ResultCode result = Builder::buildShaderPrograms(*Database, DescriptionInfo, CSharpToNativeIntermediateLanguage(IntermediateLanguage));
    return result == RecluseResult_Ok;
}


void ShaderProgramBuilder::Clear()
{
    DescriptionInfo->descriptions.clear();
    DescriptionInfo->shaderProgramIds.clear();
}


const char* ShaderProgramDescription::MakeNativeString(String^ Str)
{
    if (Str && (Str->Length != 0))
    {
        String^ const CSharpString = (String^ const)Str;
        std::string str = msclr::interop::marshal_as<std::string>(CSharpString);
        Strings->push_back(str);
        return Strings->back().c_str();
    }
    return nullptr;
}


void ShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    description.language = CSharpToNativeHighLanguage(Language);
    // TODO: Need to support permutations for interop...
}


void RasterShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    ShaderProgramDescription::StoreIn(description);
    description.pipelineType = BindType_Graphics;
    description.graphics.ps = MakeNativeString(PS);
    description.graphics.psName = MakeNativeString(PSName);
}


void VertexRasterShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    RasterShaderProgramDescription::StoreIn(description);
    description.graphics.vs = MakeNativeString(VS);
    description.graphics.vsName = MakeNativeString(VSName);

    description.graphics.gs = MakeNativeString(GS);
    description.graphics.gsName = MakeNativeString(GSName);

    description.graphics.ds = MakeNativeString(DS);
    description.graphics.dsName = MakeNativeString(DSName);

    description.graphics.hs = MakeNativeString(HS);
    description.graphics.hsName= MakeNativeString(HSName);
}


void MeshShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    RasterShaderProgramDescription::StoreIn(description);

    description.graphics.usesMeshShaders = true;
    
    description.graphics.as = MakeNativeString(AS);
    description.graphics.asName = MakeNativeString(ASName);
    
    description.graphics.ms = MakeNativeString(MS);
    description.graphics.msName = MakeNativeString(MSName);
}


void ComputeShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    ShaderProgramDescription::StoreIn(description);
    description.pipelineType = BindType_Compute;

    description.compute.cs = MakeNativeString(CS);
    description.compute.csName = MakeNativeString(CSName);
}


void RayTracingShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    ShaderProgramDescription::StoreIn(description);
    description.pipelineType = BindType_RayTrace;

    description.raytrace.rany = MakeNativeString(RAny);
    description.raytrace.ranyName = MakeNativeString(RAnyName);

    description.raytrace.rclosest = MakeNativeString(RClosest);
    description.raytrace.rclosestName = MakeNativeString(RClosestName);

    description.raytrace.rintersect = MakeNativeString(RIntersect);
    description.raytrace.rintersectName = MakeNativeString(RIntersectName);

    description.raytrace.rmiss = MakeNativeString(RMiss);
    description.raytrace.rmissName = MakeNativeString(RMissName);

    description.raytrace.rgen = MakeNativeString(RGen);
    description.raytrace.rgenName = MakeNativeString(RGenName);
}


ShaderProgramDescription::~ShaderProgramDescription()
{
    if (Strings)
    {
        delete Strings;
    }
    Strings = nullptr;
}


ShaderProgramDescription::!ShaderProgramDescription()
{
    if (Strings)
    {
        delete Strings;
    }
    Strings = nullptr;
}


bool ShaderProgramBuilder::LoadToRuntime(CSharp::IGraphicsDevice^ Device)
{
    ResultCode result = Runtime::buildAllShaderPrograms(Device(), *Database);
    return (result == RecluseResult_Ok);
}
} // Pipeline
} // CSharp
} // Recluse