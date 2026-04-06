//
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Graphics/ShaderBuilderCommon.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Messaging.hpp"

#include <sstream>
#include <regex>
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
        const Config& config,
        ShaderDebug* shaderDebugOut,
        const std::vector<PreprocessDefine>& defines
    )
{
    ResultCode result              = RecluseResult_Ok;

    std::vector<char> srcCodeString;
    std::vector<char> byteCodeString;
    srcCodeString.resize(sourceCodeBytes + 1);
    
    memcpy(srcCodeString.data(), sourceCode, sourceCodeBytes);
    srcCodeString[sourceCodeBytes] = '\0';

    result = preprocessInputResources(config.shaderLanguage, srcCodeString);

    if (!m_preprocessors.empty())
    {
        for (ShaderPreprocessor* preprocessor : m_preprocessors)
        {
            R_ASSERT(preprocessor != NULL);
            result = preprocessor->process(srcCodeString.data(), srcCodeString.size());
            if (result == RecluseResult_Ok)
            {
                // replace with new output.
                {
                    const std::vector<char>& output = preprocessor->getOutput();
                    srcCodeString.resize(output.size());
                    std::copy(output.begin(), output.end(), srcCodeString.begin());
                }
                // Release preprocessor and clear any results.
                preprocessor->release();
            }
        }
    }

    result = onCompile(srcCodeString, byteCodeString, entryPoint, config, shaderDebugOut, defines);

    if (result == RecluseResult_Ok) 
    {
        pShader->load(entryPoint, byteCodeString.data(), byteCodeString.size(), config.intermediateCode, config.shaderType);
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


ResultCode ShaderBuilder::addSystemPath(const std::string& path)
{
    Hash64 hash = recluseHashFast(path.c_str(), path.size());
    auto& it = m_systemHeaderPaths.find(hash);
    if (it == m_systemHeaderPaths.end())
    {
        m_systemHeaderPaths.insert(std::make_pair(hash, path));
        return RecluseResult_Ok;
    }
    return RecluseResult_AlreadyExists;
}


ResultCode ShaderBuilder::addLocalPath(const std::string& path)
{
    Hash64 hash = recluseHashFast(path.c_str(), path.size());
    auto& it = m_localHeaderPaths.find(hash);
    if (it == m_localHeaderPaths.end())
    {
        m_localHeaderPaths.insert(std::make_pair(hash, path));
        return RecluseResult_Ok;
    }
    return RecluseResult_AlreadyExists;
}


ResultCode ShaderBuilder::removeSystemPath(const std::string& path)
{
    Hash64 hash = recluseHashFast(path.c_str(), path.size());
    auto& it = m_systemHeaderPaths.find(hash);
    if (it != m_systemHeaderPaths.end())
    {
        m_systemHeaderPaths.erase(it);
        return RecluseResult_Ok;
    }
    return RecluseResult_NotFound;
}


ResultCode ShaderBuilder::removeLocalPath(const std::string& path)
{
    Hash64 hash = recluseHashFast(path.c_str(), path.size());
    auto& it = m_localHeaderPaths.find(hash);
    if (it != m_localHeaderPaths.end())
    {
        m_localHeaderPaths.erase(it);
        return RecluseResult_Ok;
    }
    return RecluseResult_NotFound;
}


void ShaderBuilder::clearSystemPaths()
{
    m_systemHeaderPaths.clear();
}


void ShaderBuilder::clearLocalPaths()
{
    m_localHeaderPaths.clear();
}


ShaderBuilder* createShaderBuilder(const std::string& nameID)
{
    auto& iter = g_shaderBuilderFuncs.find(nameID);
    if (iter != g_shaderBuilderFuncs.end())
    {
        ShaderBuilder* builder = iter->second();
        return builder;
    }
    R_WARN("ShaderBuilder", "Failed to create a proper shader builder! Name given = %s", nameID.c_str());
    return nullptr;
}


ResultCode ShaderPreprocessor::process(const char* sourceCodeData, U32 sourceCodeSizeBytes)
{
    R_ASSERT(sourceCodeSizeBytes != 0);
    if (sourceCodeSizeBytes == 0) return RecluseResult_InvalidArgs;
    
    return onProcess(m_output, sourceCodeData, sourceCodeSizeBytes);
}


ResultCode HlslToGlslPreprocessor::onProcess(std::vector<char>& output, const char* sourceCodeString, U32 sourceCodeSizeBytes)
{
    // Attribute for hlsl uses vk::binding(binding : int, set : int) format
    // when matching to vulkan spirv.
    // https://docs.shader-slang.org/en/stable/external/core-module-reference/attributes/vk_binding.html
    static std::string kBindingStr = "vk::binding";
    std::stringstream stream(sourceCodeString);
    std::string token;

    std::regex re(R"(register\(\s*([a-zA-Z][0-9]+)\s*(?:,\s*space\s*([0-9]+))?\s*\))");

    std::vector<std::string> sourceCodeLines;
    struct Set {
        uint numCbvs = 0;
        uint numSrvs = 0;
        uint numUavs = 0;
        uint numSamplers = 0;
        uint cbvOffset = 0;
        uint srvOffset = 0;
        uint uavOffset = 0;
        uint samplerOffset = 0;
    };
    std::map<char, Set> sets;
    std::vector<uint> resourceLines;

    U32 lineN = 0;
    while (std::getline(stream, token, '\n'))
    {
        //R_NOTIFY("HLSL To GLSL", "%s", token.c_str());
        std::smatch match;
        if ((token.size() > 2) && token[0] != '/' && token[1] != '/')
        {
            if (std::regex_search(token, match, re))
            {
                resourceLines.push_back(lineN);
                char space = match[2].matched ? match[2].str()[0] : '0'; // may be empty
                char registerType = match[1].str()[0];
                char registerNum = match[1].str()[1];

                // Count the number of shader resources.
                if (registerType == 'b')
                {
                    sets[space].numCbvs += 1;
                    sets[space].cbvOffset = Math::maximum<uint>(registerNum - '0' + 1, sets[space].cbvOffset);
                }
                else if (registerType == 't')
                {
                    sets[space].numSrvs += 1;
                    sets[space].srvOffset = Math::maximum<uint>(registerNum - '0' + 1, sets[space].srvOffset);
                }
                else if (registerType == 's')
                {
                    sets[space].numSamplers += 1;
                    sets[space].samplerOffset = Math::maximum<uint>(registerNum - '0' + 1, sets[space].samplerOffset);
                }
                else if (registerType == 'u')
                {
                    sets[space].numUavs += 1;
                    sets[space].uavOffset = Math::maximum<uint>(registerNum - '0' + 1, sets[space].uavOffset);
                }
            }
        }
        token.push_back('\n');
        sourceCodeLines.push_back(token);
        lineN++;
        //R_NOTIFY("HLSL To GLSL", "%s", token.c_str());
    }

    for (uint resI : resourceLines)
    {   
        std::string line = sourceCodeLines[resI];
        std::smatch match;
        if (std::regex_search(line, match, re))
        {
            std::string space = match[2].matched ? match[2].str() : "0"; // may be empty
            char registerType = match[1].str()[0];
            uint offset = 0;
            if (registerType == 'b')
                offset = 0;
            else if (registerType == 't')
                offset = sets[space[0]].cbvOffset;
            else if (registerType == 'u')
                offset =  sets[space[0]].cbvOffset + sets[space[0]].srvOffset;
            else if (registerType == 's')
                offset = sets[space[0]].cbvOffset + sets[space[0]].srvOffset + sets[space[0]].uavOffset;
            uint index = match[1].str()[1] - '0';
            index = offset + index;
            line = "[[" + kBindingStr + "(" + std::to_string(index) + ", " + space + ")]] " + line;
            //R_NOTIFY("HLSL to GLSL", "Register %s, Space %s", match[1].str().c_str(), space.c_str());
        }
        sourceCodeLines[resI] = line;
    }

    // Copy to output.
    for (uint i = 0; i < sourceCodeLines.size(); ++i)
    {

        if (m_debug)
        {
            R_NOTIFY("HLSL To GLSL", "%s", sourceCodeLines[i].c_str());
        }

        for (char c : sourceCodeLines[i])
        {
            output.push_back(c);
        }
    }
    output.push_back('\0');
    return RecluseResult_Ok;
}


ResultCode GlslToHlslPreprocessor::onProcess(std::vector<char>& output, const char* sourceCodeString, U32 sourceCodeSizeBytes)
{
    return RecluseResult_Ok;
}
} // Pipeline
} // Recluse