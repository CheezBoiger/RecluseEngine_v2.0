//
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"

#include "Recluse/Messaging.hpp"

#include "Recluse/Threading/Threading.hpp"

namespace Recluse {

ShaderId kShaderCounter = 0;
MutexGuard kShaderCounterMutex = MutexGuard("ShaderCounterMutex");

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


Hash64 Shader::makeShaderHash(const char* pBytecode, U64 sizeBytes)
{
    return recluseHashFast(pBytecode, sizeBytes);
}


ResultCode Shader::load(const char* entryPoint, const char* pByteCode, U64 szBytes, ShaderIntermediateCode imm, ShaderType shaderType)
{
    m_byteCode.resize(szBytes);
    memcpy(m_byteCode.data(), pByteCode, szBytes);

    m_intermediateCode  = imm;
    m_shaderType        = shaderType;
    m_entryPoint        = entryPoint;

    if (m_entryPoint.back() != '\0')
    {
        m_entryPoint.append("\0");
    }

    m_shaderHashId = Shader::makeShaderHash(pByteCode, szBytes);

    return RecluseResult_Ok;
}


Shader* Shader::convertTo(ShaderIntermediateCode intermediateCode)
{
    return nullptr;
}


ResultCode Shader::saveToFile(const char* filePath)
{
    ResultCode result = RecluseResult_Ok;
    FileBufferData data = { };
    data.resize(m_byteCode.size());
    std::copy(m_byteCode.begin(), m_byteCode.end(), data.begin());

    return File::writeTo(&data, std::string(filePath));
}


ResultCode Shader::serialize(Archive* archive)
{
    archive->write(&m_shaderHashId, sizeof(Hash64));
    archive->write(&m_shaderNameHash, sizeof(ShaderId));
    archive->write(&m_shaderType, sizeof(ShaderType));
    archive->write(&m_intermediateCode, sizeof(ShaderIntermediateCode));
    archive->write(&m_permutation, sizeof(ShaderPermutationId));

    U32 nameSize = m_shaderName.size();
    U32 entryPointLenBytes = m_entryPoint.size();
    U32 bytecodeSize = m_byteCode.size();

    archive->write(&nameSize, sizeof(U32));
    archive->write(&entryPointLenBytes, sizeof(U32));
    archive->write(&bytecodeSize, sizeof(U32));

    archive->write((void*)m_shaderName.data(), sizeof(char) * m_shaderName.size());
    archive->write((void*)m_entryPoint.data(), sizeof(char) * entryPointLenBytes);
    archive->write((void*)m_byteCode.data(), sizeof(char) * m_byteCode.size());

    return RecluseResult_Ok;
}


ResultCode Shader::deserialize(Archive* archive)
{
    archive->read(&m_shaderHashId, sizeof(Hash64));
    archive->read(&m_shaderNameHash, sizeof(ShaderId));
    archive->read(&m_shaderType, sizeof(ShaderType));
    archive->read(&m_intermediateCode, sizeof(ShaderIntermediateCode));
    archive->read(&m_permutation, sizeof(ShaderPermutationId));

    U32 nameSize = 0;
    U32 entryPointLenBytes = 0;
    U32 bytecodeSize = 0;

    archive->read(&nameSize, sizeof(U32));
    archive->read(&entryPointLenBytes, sizeof(U32));
    archive->read(&bytecodeSize, sizeof(U32));
    
    m_shaderName.resize(nameSize);
    m_entryPoint.resize(entryPointLenBytes);
    m_byteCode.resize(bytecodeSize);

    archive->read((void*)m_shaderName.data(), sizeof(char) * nameSize);
    archive->read((void*)m_entryPoint.data(), sizeof(char) * entryPointLenBytes);
    archive->read((void*)m_byteCode.data(), sizeof(char) * bytecodeSize);

    return RecluseResult_Ok;
}
} // Recluse