//
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Graphics/ShaderBuilderCommon.hpp"
#include "Recluse/Messaging.hpp"

#include <unordered_map>

namespace Recluse {
namespace Pipeline {


ResultCode ShaderBuilder::compile
    (
        Shader* pShader, 
        const char* entryPoint,
        const char* sourceCode, 
        U64 sourceCodeBytes, 
        ShaderPermutationId permutation,
        ShaderLang lang, 
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

    result = onCompile(srcCodeString, byteCodeString, entryPoint, lang, shaderType, defines);

    if (result == RecluseResult_Ok) 
    {
        pShader->load(entryPoint, byteCodeString.data(), byteCodeString.size(), getIntermediateCode(), shaderType);
        pShader->setPermutationId(permutation);
    } 
    else 
    {
        R_ERROR("Shader", "Failed to compile shader!");
    }
    return result;
}


std::unordered_map<std::string, ShaderBuilderFunc> g_shaderBuilderFuncs = 
{ 
    { "glslang",    createGlslangShaderBuilder },
    { "dxc",        createDxcShaderBuilder }
};


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