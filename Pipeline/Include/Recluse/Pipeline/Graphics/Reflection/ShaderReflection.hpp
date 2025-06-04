//
#pragma once

#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"

namespace Recluse {
namespace Pipeline {


// Shader Reflection object, used to reflect a given shader.
class ReclusePipeline_PUBLIC_API ShaderReflection
{
public:
    ShaderReflection(ShaderIntermediateCode code)
        : m_intermediateCode(code) { }

    virtual ~ShaderReflection() { }
    virtual ResultCode reflect(ShaderReflectionInformation& info, const Shader* shader) { return RecluseResult_NoImpl; } 

    ShaderIntermediateCode getIntermediateCode() const { return m_intermediateCode; }
private:
    ShaderIntermediateCode m_intermediateCode;
};


ReclusePipeline_PUBLIC_API ShaderReflection* createShaderReflection(ShaderIntermediateCode intermediateCode);
} // Pipeline
} // Recluse