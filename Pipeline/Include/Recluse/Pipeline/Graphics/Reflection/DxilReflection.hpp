//
#pragma once

#include "Recluse/Pipeline/Graphics/Reflection/ShaderReflection.hpp"

namespace Recluse {
namespace Pipeline {


class ReclusePipeline_PUBLIC_API DxilReflection : public ShaderReflection
{
public:
    DxilReflection() : ShaderReflection(ShaderIntermediateCode_Dxil) { }

    ResultCode reflect(ShaderReflectionInformation& infoOut, const Shader* shader) override;
};
} // Pipeline
} // Recluse