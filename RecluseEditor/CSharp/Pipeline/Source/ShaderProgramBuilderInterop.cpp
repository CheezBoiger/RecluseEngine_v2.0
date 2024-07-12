//
#include "ShaderProgramBuilderInterop.hpp"

namespace Recluse {
namespace CSharp {
namespace Pipeline {


ShaderProgramBuilder::ShaderProgramBuilder()
{
}


bool ShaderProgramBuilder::PushDescription(ShaderProgramDescription^ Description)
{
    return false;
}


bool ShaderProgramBuilder::SaveToDisk()
{
    return false;
}


bool ShaderProgramBuilder::LoadFromDisk()
{
    return false;
}


bool ShaderProgramBuilder::Build(ShaderIntermediateLanguage IntermediateLanguage)
{
    return false;
}


void ShaderProgramBuilder::Clear()
{
}
} // Pipeline
} // CSharp
} // Recluse