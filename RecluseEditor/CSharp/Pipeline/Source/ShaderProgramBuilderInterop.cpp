//
#include "ShaderProgramBuilderInterop.hpp"
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Messaging.hpp"

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


Recluse::BindType CSharpToNativePipelineType(PipelineType Pipeline)
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


ShaderProgramBuilder::ShaderProgramBuilder()
    : Database(nullptr)
    , DescriptionInfo(nullptr)
{
    Database = new ShaderProgramDatabase();
    DescriptionInfo = new ShaderProgramDescriptionInfo();
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


void ShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    description.language = CSharpToNativeHighLanguage(Language);
    // TODO: Need to support permutations for interop...
}


void RasterShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    ShaderProgramDescription::StoreIn(description);
    description.pipelineType = BindType_Graphics;
}


void VertexRasterShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    RasterShaderProgramDescription::StoreIn(description);
}


void MeshShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    RasterShaderProgramDescription::StoreIn(description);
}


void ComputeShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    ShaderProgramDescription::StoreIn(description);
    description.pipelineType = BindType_Compute;
}


void RayTracingShaderProgramDescription::StoreIn(Builder::ShaderProgramDescription& description)
{
    ShaderProgramDescription::StoreIn(description);
    description.pipelineType = BindType_RayTrace;
}


ShaderProgramDescription::~ShaderProgramDescription()
{
}


bool ShaderProgramBuilder::LoadToRuntime(CSharp::IGraphicsDevice^ Device)
{
    ResultCode result = Runtime::buildAllShaderPrograms(Device(), *Database);
    return (result == RecluseResult_Ok);
}
} // Pipeline
} // CSharp
} // Recluse