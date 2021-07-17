//
#include "Recluse/Graphics/Shader.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include "Recluse/Messaging.hpp"


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


ErrType Shader::load(char* pByteCode, U64 szBytes)
{
    Hash64 hash = recluseHash(pByteCode, szBytes);
    
    m_pByteCode = pByteCode;
    m_szBytes = szBytes;
    m_crc = hash;

    return REC_RESULT_OK;
}


ErrType Shader::compile(char* sourceCode, U64 sourceCodeBytes)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


Shader* Shader::convertTo(ShaderIntermediateCode intermediateCode)
{
    return nullptr;
}


U32 Shader::disassemble(char* disassembledCode)
{
    return 0;
}
} // Recluse