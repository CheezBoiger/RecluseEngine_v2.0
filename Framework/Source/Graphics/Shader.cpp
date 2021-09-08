//
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include "Recluse/Messaging.hpp"

#include "Recluse/Graphics/ShaderBuilder.hpp"

namespace Recluse {


Shader* Shader::create()
{
    return new Shader();
}


void Shader::destroy(Shader* pShader)
{
    if (pShader) {
        delete pShader;
    }
}


void Shader::genCrc()
{
    Hash64 hash = recluseHash(m_byteCode.data(), m_byteCode.size());
    m_crc = hash;
}


ErrType Shader::load(const char* pByteCode, U64 szBytes, ShaderIntermediateCode imm, ShaderType shaderType)
{
    m_byteCode.resize(szBytes);
    memcpy(m_byteCode.data(), pByteCode, szBytes);

    m_intermediateCode  = imm;
    m_shaderType        = shaderType;

    genCrc();

    return REC_RESULT_OK;
}


ErrType ShaderBuilder::compile(Shader* pShader, const char* sourceCode, U64 sourceCodeBytes, 
    ShaderLang lang, ShaderType shaderType)
{
    ErrType result              = REC_RESULT_OK;

    std::vector<char> srcCodeString;
    std::vector<char> byteCodeString;
    srcCodeString.resize(sourceCodeBytes + 1);
    
    memcpy(srcCodeString.data(), sourceCode, sourceCodeBytes);
    srcCodeString[sourceCodeBytes] = '\0';    

    result = onCompile(srcCodeString, byteCodeString, lang, shaderType);

    if (result == REC_RESULT_OK) {

        pShader->load(byteCodeString.data(), byteCodeString.size(), getIntermediateCode(), shaderType);
    
    } else {

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
    ErrType result = REC_RESULT_OK;
    FileBufferData data = { };
    data.buffer.resize(m_byteCode.size());
    std::copy(m_byteCode.begin(), m_byteCode.end(), data.buffer.begin());

    return File::writeTo(&data, std::string(filePath));
}
} // Recluse