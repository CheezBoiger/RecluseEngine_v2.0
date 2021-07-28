//
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include "Recluse/Messaging.hpp"

#include "ShaderBuilder.hpp"

namespace Recluse {


Shader* Shader::create(ShaderIntermediateCode code, ShaderType type)
{
    return new Shader(code, type);
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


ErrType Shader::load(char* pByteCode, U64 szBytes)
{
    m_byteCode.resize(szBytes);
    memcpy(m_byteCode.data(), pByteCode, szBytes);

    genCrc();

    return REC_RESULT_OK;
}


ErrType Shader::compile(char* sourceCode, U64 sourceCodeBytes, ShaderLang lang)
{
    ErrType result              = REC_RESULT_OK;
    ShaderBuilder* pBuilder     = nullptr;

    std::vector<char> srcCodeString;
    srcCodeString.resize(sourceCodeBytes + 1);
    
    memcpy(srcCodeString.data(), sourceCode, sourceCodeBytes);
    srcCodeString[sourceCodeBytes] = '\0';

    // Using glslang compiler to handle both hlsl and glsl compilation to spirv.
    // Use DXC compiler to handle both hlsl and hlsl to DXIL or DXBC.
    if (getIntermediateCodeType() == INTERMEDIATE_SPIRV) {
        pBuilder = createGlslangShaderBuilder(m_shaderType, getIntermediateCodeType());
    } else {
        pBuilder = createDxcShaderBuilder(m_shaderType, getIntermediateCodeType());
    }

    if (!pBuilder) {

        R_ERR("Shader", "Could not instantiate a proper shader builder.");

        return REC_RESULT_FAILED;
    }

    result = pBuilder->compile(srcCodeString, m_byteCode, lang);        

    if (result == REC_RESULT_OK) {

        genCrc();
    
    } else {

        R_ERR("Shader", "Failed to compile shader!");

    }

    freeShaderBuilder(pBuilder);

    return result;
}


Shader* Shader::convertTo(ShaderIntermediateCode intermediateCode)
{
    return nullptr;
}


U32 Shader::disassemble(char* disassembledCode)
{
    return 0;
}


ErrType Shader::saveToFile(const char* filePath)
{
    ErrType result = REC_RESULT_OK;
    File file = { };
    file.data.resize(m_byteCode.size());
    std::copy(m_byteCode.begin(), m_byteCode.end(), file.data.begin());

    return File::writeTo(&file, std::string(filePath));
}
} // Recluse