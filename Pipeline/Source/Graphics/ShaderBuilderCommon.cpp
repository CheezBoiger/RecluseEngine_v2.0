//
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Graphics/ShaderBuilderCommon.hpp"
#include "Recluse/Messaging.hpp"

#include <unordered_map>

namespace Recluse {
namespace Pipeline {


// Shader Builder Functions.
std::unordered_map<std::string, ShaderBuilderFunc> g_shaderBuilderFuncs = 
{ 
    { "glslang",    createGlslangShaderBuilder },
    { "dxc",        createDxcShaderBuilder }
};


ResultCode ShaderBuilder::compile
    (
        Shader* pShader, 
        const char* entryPoint,
        const char* sourceCode, 
        U64 sourceCodeBytes,
        ShaderLanguage lang, 
        ShaderType shaderType,
        const std::vector<PreprocessDefine>& defines
    )
{
    ResultCode result              = RecluseResult_Ok;

    std::vector<char> srcCodeString;
    std::vector<char> byteCodeString;
    srcCodeString.resize(sourceCodeBytes + 1);
    
    memcpy(srcCodeString.data(), sourceCode, sourceCodeBytes);
    srcCodeString[sourceCodeBytes] = '\0';

    result = preprocessInputResources(lang, srcCodeString);

    result = onCompile(srcCodeString, byteCodeString, entryPoint, lang, shaderType, defines);

    if (result == RecluseResult_Ok) 
    {
        pShader->load(entryPoint, byteCodeString.data(), byteCodeString.size(), getIntermediateCode(), shaderType);
        Hash64 permutationId = recluseHashFast(byteCodeString.data(), byteCodeString.size());
        pShader->setPermutationId(permutationId);
    } 
    else 
    {
        R_ERROR("Shader", "Failed to compile shader!");
    }
    return result;
}


ResultCode ShaderBuilder::preprocessInputResources(ShaderLanguage lang, std::vector<char>& srcCodeString)
{
    // Common shader builder doesn't need to do anything fancy, best to leave it for 
    // specialized shader builders.
    return RecluseResult_Ok;
}


ShaderBuilder* createShaderBuilder(const std::string& nameID, ShaderIntermediateCode intermediateCode)
{
    auto& iter = g_shaderBuilderFuncs.find(nameID);
    if (iter != g_shaderBuilderFuncs.end())
    {
        ShaderBuilder* builder = iter->second(intermediateCode);
        return builder;
    }
    R_WARN("ShaderBuilder", "Failed to create a proper shader builder! Name given = %s", nameID.c_str());
    return nullptr;
}
} // Pipeline
} // Recluse