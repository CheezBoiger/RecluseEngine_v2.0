//
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"

#include "Recluse/Messaging.hpp"

#include "Recluse/Threading/Threading.hpp"

#include <map>
#include <memory>

namespace Recluse {

ShaderId kShaderCounter = 0;
MutexGuard kShaderCounterMutex = MutexGuard("ShaderCounterMutex");
std::map<Hash64, std::unique_ptr<Shader>> kActiveShaderMap; 

Shader* Shader::create()
{
    ScopedLock lck(kShaderCounterMutex);
    std::unique_ptr<Shader> shader = std::unique_ptr<Shader>(new Shader());
    shader->m_instanceId = kShaderCounter++;
    ShaderId instanceId = shader->getInstanceId();
    auto iter = kActiveShaderMap.find(instanceId);
    if (iter == kActiveShaderMap.end())
    {
        kActiveShaderMap.insert(std::make_pair(instanceId, std::move(shader)));
    }
    else
    {
        shader->release();
    }
    return kActiveShaderMap[instanceId].get();
}


void Shader::destroy(Shader* pShader)
{
    if (pShader) 
    {
        ShaderId instanceId = pShader->getInstanceId();
        auto iter = kActiveShaderMap.find(instanceId);
        if (iter != kActiveShaderMap.end())
        {
            ScopedLock _(kShaderCounterMutex);
            kActiveShaderMap[instanceId]->release();
            kActiveShaderMap.erase(iter);
        }
    }
}


Hash64 Shader::makeShaderHash(const char* pBytecode, U64 sizeBytes)
{
    return recluseHashFast(pBytecode, sizeBytes);
}


ResultCode Shader::load(const char* entryPoint, const char* pByteCode, U64 szBytes, ShaderIntermediateCode imm, ShaderType shaderType)
{
    R_ASSERT(entryPoint);
    R_ASSERT(pByteCode);
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
    if (filePath)
    {
        ResultCode result = RecluseResult_Ok;
        FileBufferData data = { };
        data.resize(m_byteCode.size());
        std::copy(m_byteCode.begin(), m_byteCode.end(), data.begin());
        return File::writeTo(&data, std::string(filePath));
    }
    return RecluseResult_NullPtrExcept;
}


ResultCode Shader::serialize(Archive* archive) const
{
    R_ASSERT(archive);
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
    R_ASSERT(archive);
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


ResultCode ShaderReflection::serialize(Archive* archive) const
{
    R_ASSERT(archive);
    archive->write(&metadata, sizeof(metadata));
    archive->write(cbvs.data(), sizeof(ReflectionBind) * metadata.numCbvs);
    archive->write(srvs.data(), sizeof(ReflectionBind) * metadata.numSrvs);
    archive->write(uavs.data(), sizeof(ReflectionBind) * metadata.numUavs);
    archive->write(samplers.data(), sizeof(ReflectionBind) * metadata.numSamplers);
    return RecluseResult_Ok;
}


ResultCode ShaderReflection::deserialize(Archive* archive)
{
    R_ASSERT(archive);
    archive->read(&metadata, sizeof(metadata));
    
    cbvs.resize(metadata.numCbvs);
    srvs.resize(metadata.numSrvs);
    uavs.resize(metadata.numUavs);
    samplers.resize(metadata.numSamplers);

    archive->read(cbvs.data(), sizeof(ReflectionBind) * metadata.numCbvs);
    archive->read(srvs.data(), sizeof(ReflectionBind) * metadata.numSrvs);
    archive->read(uavs.data(), sizeof(ReflectionBind) * metadata.numUavs);
    archive->read(samplers.data(), sizeof(ReflectionBind) * metadata.numSamplers);
    
    return RecluseResult_Ok;
}
} // Recluse