//
#include "Recluse/Pipeline/Graphics/Reflection/DxilReflection.hpp"
#include "Recluse/Math/MathCommons.hpp"

#include "Recluse/Messaging.hpp"

#if defined RCL_DXC 
#include <atlbase.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#endif

namespace Recluse {
namespace Pipeline {

#define DXBC_FOURCC(ch0, ch1, ch2, ch3)                                        \
  ((UINT)(BYTE)(ch0) | ((UINT)(BYTE)(ch1) << 8) | ((UINT)(BYTE)(ch2) << 16) |  \
   ((UINT)(BYTE)(ch3) << 24))

R_INTERNAL UINT32 DXBC_DXIL = DXBC_FOURCC('D', 'X', 'I', 'L');          // == DFCC_DXIL


// Packs the shader binding slot and space slot together into a 32-bit register.
R_INTERNAL U32 packShaderBind(U16 space, U16 bind)
{
    return (U32)bind | ((U32)space << 16);
}

// Shader reflection.
ResultCode DxilReflection::reflect(ShaderReflectionInformation& reflectionOutput, const Shader* shader)
{
#if RCL_DXC
    const char* bytecode = shader->getByteCode();
    U64 sizeBytes = shader->getSzBytes();
    CComPtr<IDxcContainerReflection> containerReflection;
    CComPtr<ID3D12ShaderReflection> shaderReflection;
    UINT32 shaderIndex;
        
    HRESULT hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&containerReflection));
    CComPtr<ID3DBlob> blob;
    if (FAILED(hr))
    {
        R_ERROR("DXC", "Failed to create container reflection!");
        return RecluseResult_Failed;
    }
    hr = D3DCreateBlob(sizeBytes, &blob);
    if (FAILED(hr))
    {
        R_ERROR("DXC", "Failed to create blob for reflection!");
        return RecluseResult_Failed;
    }
    memcpy(blob->GetBufferPointer(), bytecode, sizeBytes);
    hr = containerReflection->Load((IDxcBlob*)blob.p);
    if (FAILED(hr))
    {
        R_ERROR("DXC", "Failed to properly reflect shader!");
        return RecluseResult_Failed;
    }
    hr = containerReflection->FindFirstPartKind(DXBC_DXIL, &shaderIndex);
    R_ASSERT(SUCCEEDED(hr));
    containerReflection->GetPartReflection(shaderIndex, __uuidof(ID3D12ShaderReflection), (void**)&shaderReflection);
    R_ASSERT(SUCCEEDED(hr));
    D3D12_SHADER_DESC shaderDesc = { };
    shaderReflection->GetDesc(&shaderDesc);
    // Shader reflection may not be accurate in what is actually bound to the shader. Any constant buffers not used
    // will be optimized out, but still be incremented in this var.
    // reflectionOutput.metadata.numCbvs = shaderDesc.ConstantBuffers;

    U32 numResources = shaderDesc.BoundResources;
    for (U32 resourceIdx = 0; resourceIdx < numResources; ++resourceIdx)
    {
        D3D12_SHADER_INPUT_BIND_DESC shaderInputDesc = { };
        shaderReflection->GetResourceBindingDesc(resourceIdx, &shaderInputDesc);

        U32 space = shaderInputDesc.Space;
        UINT bindRange = shaderInputDesc.BindPoint + shaderInputDesc.BindCount;
            
        if (space >= reflectionOutput.perSetMetadata.size())
            reflectionOutput.perSetMetadata.resize(space + 1);
        ShaderReflectionInformation::Metadata& metadata = reflectionOutput.perSetMetadata[space];

        // For DXC, we store the register bind. (c<BindPoint>, c<BindPoint+1>, c<BindPoint+2>, c<BindPoint+3> ...)
        // This will store all bind points from HLSL -> D3D12.
        switch (shaderInputDesc.Type)
        {
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
            {
                metadata.baseCbv = Math::minimum(metadata.baseCbv, static_cast<U8>(shaderInputDesc.BindPoint));
                for (UINT bind = shaderInputDesc.BindPoint; bind < bindRange; ++bind)
                {
                    reflectionOutput.cbvs.push_back(static_cast<ShaderBind>(packShaderBind(space, bind)));
                    metadata.numCbvs += 1;
                }
                break;
            }
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED:
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_BYTEADDRESS:
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_TBUFFER:
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
            {
                metadata.baseSrv = Math::minimum(metadata.baseSrv, static_cast<U8>(shaderInputDesc.BindPoint));
                for (UINT bind = shaderInputDesc.BindPoint; bind < bindRange; ++bind)
                {
                    reflectionOutput.srvs.push_back(static_cast<ShaderBind>(packShaderBind(space, bind)));                    
                    metadata.numSrvs += 1;
                }
                break;
            }
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
            {
                metadata.baseSampler = Math::minimum(metadata.baseSampler, static_cast<U8>(shaderInputDesc.BindPoint));
                for (UINT bind = shaderInputDesc.BindPoint; bind < bindRange; ++bind)
                {
                    reflectionOutput.samplers.push_back(static_cast<ShaderBind>(packShaderBind(space, bind)));                    
                    metadata.numSamplers += 1;
                }
                break;
            }
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_FEEDBACKTEXTURE:
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED:
            {
                metadata.baseUav = Math::minimum(metadata.baseUav, static_cast<U8>(shaderInputDesc.BindPoint));
                for (UINT bind = shaderInputDesc.BindPoint; bind < bindRange; ++bind)
                {
                    reflectionOutput.uavs.push_back(static_cast<ShaderBind>(packShaderBind(space, bind)));                    
                    metadata.numUavs += 1;
                }
                break;
            }
        }
    }
    return RecluseResult_Ok;
#else
    return RecluseResult_NoImpl;
#endif
}
} // Pipeline
} // Recluse