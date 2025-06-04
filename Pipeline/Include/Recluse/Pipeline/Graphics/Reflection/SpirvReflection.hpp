//
#pragma once

#include "Recluse/Pipeline/Graphics/Reflection/ShaderReflection.hpp"

namespace Recluse {
namespace Pipeline {


class SpirvReflection : public ShaderReflection
{
public:
    SpirvReflection() : ShaderReflection(ShaderIntermediateCode_Spirv) { }
    ~SpirvReflection() { }

    ResultCode reflect(ShaderReflectionInformation& info, const Shader* shader) override;
    
};
} // Pipeline
} // Recluse