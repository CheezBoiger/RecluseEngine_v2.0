//
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include "Recluse/Messaging.hpp"

#include "Recluse/Graphics/ShaderBuilder.hpp"
#include "Recluse/Threading/Threading.hpp"

namespace Recluse {

ShaderId kShaderCounter = 0;
Mutex kShaderCounterMutex = createMutex("ShaderCounterMutex");

Shader* Shader::create()
{
    return new Shader();
}


void Shader::destroy(Shader* pShader)
{
    if (pShader) 
    {
        delete pShader;
    }
}


void Shader::genHashId()
{
    // Only generate the id if it doesn't already have one.
    if (kShaderCounter == ~0)
    { 
        ScopedLock _(kShaderCounterMutex);
        m_uniqueId = kShaderCounter++;
    }
}


ErrType Shader::load(const char* entryPoint, const char* pByteCode, U64 szBytes, ShaderIntermediateCode imm, ShaderType shaderType)
{
    m_byteCode.resize(szBytes);
    memcpy(m_byteCode.data(), pByteCode, szBytes);

    m_intermediateCode  = imm;
    m_shaderType        = shaderType;
    m_entryPoint        = entryPoint;

    genHashId();

    return RecluseResult_Ok;
}


ErrType ShaderBuilder::compile
    (
        Shader* pShader, 
        const char* entryPoint,
        const char* sourceCode, 
        U64 sourceCodeBytes, 
        ShaderLang lang, 
        ShaderType shaderType,
        const std::vector<PreprocessDefine>& defines
    )
{
    ErrType result              = RecluseResult_Ok;

    std::vector<char> srcCodeString;
    std::vector<char> byteCodeString;
    srcCodeString.resize(sourceCodeBytes + 1);
    
    memcpy(srcCodeString.data(), sourceCode, sourceCodeBytes);
    srcCodeString[sourceCodeBytes] = '\0';    

    result = onCompile(srcCodeString, byteCodeString, entryPoint, lang, shaderType, defines);

    if (result == RecluseResult_Ok) 
    {
        pShader->load(entryPoint, byteCodeString.data(), byteCodeString.size(), getIntermediateCode(), shaderType);
    } 
    else 
    {
        R_ERR("Shader", "Failed to compile shader!");
    }

    return result;
}


Shader* Shader::convertTo(ShaderIntermediateCode intermediateCode)
{
    return nullptr;
}


ErrType Shader::saveToFile(const char* filePath)
{
    ErrType result = RecluseResult_Ok;
    FileBufferData data = { };
    data.resize(m_byteCode.size());
    std::copy(m_byteCode.begin(), m_byteCode.end(), data.begin());

    return File::writeTo(&data, std::string(filePath));
}
} // Recluse