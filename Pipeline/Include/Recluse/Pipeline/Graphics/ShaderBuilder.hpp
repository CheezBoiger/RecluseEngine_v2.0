//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include "ReclusePipeline_exports.hpp"

namespace Recluse {
namespace Pipeline {

struct PreprocessDefine
{
    std::string variable;
    std::string value;
};


// ShaderBuilder, handles high level shading languages, and transforms them into
// bytecode to be read to the gpu.
class ReclusePipeline_PUBLIC_API ShaderBuilder 
{
public:
    // Configuration for the shader builder.
    struct Config
    {
        enum 
        {
            // Default Config 
            Default, 
            // Optimize, which is default
            Optimize = Default, 
            // Disable optimizations, intended for debugging shaders.
            Disable 
        };
        typedef u32 OptimizationOption;


        OptimizationOption  option;
        Bool                dumpSymbols;
    };

    ShaderBuilder()
        : m_builderConfig({ }) 
    { }
    virtual ~ShaderBuilder() { }
    
    // Set up the shader builder. Should be the warm up setup.
    virtual ResultCode setUp() { return RecluseResult_NoImpl; }

    // Tear down the shader builder, that was initialized. Everything 
    // that is initialized by the shaderbuilder should be cleaned up.
    virtual ResultCode tearDown() { return RecluseResult_NoImpl; }

    // Set the configuration.
    void setConfiguration(const Config& config) { m_builderConfig = config; }

    // compile the shader and return the bytecode.
    // If successful, the shader will contain the compiled bytecode and information.
    ResultCode compile
        (
            Shader* pShaderOut,
            const char* entryPoint,
            const char* srcCode, 
            U64 sourceCodeBytes,
            ShaderLanguage lang,
            ShaderType shaderType,
            ShaderIntermediateCode intermediateCode,
            const std::vector<PreprocessDefine>& defines = std::vector<PreprocessDefine>()
        );

    virtual ResultCode disassemble(std::vector<char>& output) { return RecluseResult_NoImpl; }
    
    // Is the builder for debug mode.
    Bool isDebugMode() const { return (m_builderConfig.option != Config::Disable); }

    Config::OptimizationOption getOptimizationOption() const { return m_builderConfig.option; }

private:

    // OnCompile function abstract intended to be overridden on specifying shaderbuilders.
    virtual ResultCode onCompile
                        (
                            const std::vector<char>& srcCode,
                            std::vector<char>& byteCode, 
                            const char* entryPoint,
                            ShaderLanguage lang, 
                            ShaderType shaderType,
                            ShaderIntermediateCode intermediateCode,
                            const std::vector<PreprocessDefine>& defines = std::vector<PreprocessDefine>()
                        ) 
        { return RecluseResult_NoImpl; }

    virtual ResultCode preprocessInputResources(ShaderLanguage lang, std::vector<char>& sourceCode);

    Config                  m_builderConfig;
};

// Must be newly allocated.
ReclusePipeline_PUBLIC_API ShaderBuilder* createShaderBuilder(const std::string& nameID);

// Must call when cleaning up our shader builders.
static void freeShaderBuilder(ShaderBuilder* pBuilder) 
{
    delete pBuilder;
}
} // Pipeline
} // Recluse