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


// Shader preprocessor object, used to handle shader source code files, and 
// modify if need be.
class ReclusePipeline_PUBLIC_API ShaderPreprocessor
{
public:
    // Process the source code, the result is stored and must be obtained through getOutput().
    // \param sourceCodeData the pointer to the source code.
    // \param sourceCodeSizeBytes the length of the source code pointer, in bytes.
    // \return the result code, Ok if successful.
    ResultCode process(const char* sourceCodeData, U32 sourceCodeSizeBytes);

    ShaderPreprocessor& addConstantDefine(const std::string& variableName, const std::string& value)
    {
        m_defines.push_back(PreprocessDefine{variableName, value});
        return (*this);
    }

    // Get the output of the preprocessor. This is usually if the process() call is successful.
    const std::vector<char>& getOutput() {  return m_output; }

    const std::vector<PreprocessDefine>& getDefines() const { return m_defines; }

    void release() { m_output.clear(); m_output.resize(0); }
    void setDebug(Bool enable) { m_debug = enable; }
    Bool isDebugging() const { return m_debug; }
protected:

    virtual ResultCode onProcess(std::vector<char>& out, const char* sourceCodeString, U32 sourceCodeSizeBytes) = 0;

    // Defines may be added by this preprocesor to append to the original source code.
    // Maybe an overwrite function to replace something in the shader code.
    std::vector<PreprocessDefine> m_defines;
    Bool                            m_debug = false;

private:
    std::vector<char> m_output;
};


// Handles processing of hlsl to glsl conversions.
class ReclusePipeline_PUBLIC_API HlslToGlslPreprocessor : public ShaderPreprocessor
{
public:
    ResultCode onProcess(std::vector<char>& out, const char* sourceCodeString, U32 sourceCodeSizeBytes) override;
};


// Handles processoing of glsl to hlsl conversions.
class ReclusePipeline_PUBLIC_API GlslToHlslPreprocessor : public ShaderPreprocessor
{
public:
    ResultCode onProcess(std::vector<char>& out, const char* sourceCodeString, U32 sourceCodeSizeBytes) override;
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

    ResultCode  addSystemPath(const std::string& path);
    ResultCode  addLocalPath(const std::string& path);

    ResultCode  removeSystemPath(const std::string& path);
    ResultCode  removeLocalPath(const std::string& path);

    void        clearSystemPaths();
    void        clearLocalPaths();

    void        addPreprocessor(ShaderPreprocessor* preprocessor) { m_preprocessors.push_back(preprocessor); }
    
    // Is the builder for debug mode.
    Bool isDebugMode() const { return (m_builderConfig.option != Config::Disable); }

    Config::OptimizationOption getOptimizationOption() const { return m_builderConfig .option; }

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

    std::map<Hash64, std::string> m_systemHeaderPaths;
    std::map<Hash64, std::string> m_localHeaderPaths;

    std::vector<ShaderPreprocessor*> m_preprocessors;
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