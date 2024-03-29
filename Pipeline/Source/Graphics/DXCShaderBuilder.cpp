//
#include "Recluse/Messaging.hpp"
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Recluse/Utility.hpp"
#include "Win32/Win32Common.hpp"

#include "Recluse/Messaging.hpp"

#include <codecvt>
#include <locale>

#if defined RCL_DXC 
#include <atlbase.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>

#include "dxc/DxilContainer/DxilContainer.h"
#endif

R_DECLARE_GLOBAL_STRING(g_shaderModel, "6_0", "DXC.ShaderModel");

namespace Recluse {
namespace Pipeline {


class DxcBlob : public IDxcBlob
{
public:
    virtual ~DxcBlob() 
    {
        m_blob.Release(); 
    }

    DxcBlob(const void* pData, SIZE_T sizeBytes)
    {
        HRESULT hr = D3DCreateBlob(sizeBytes, &m_blob);
        R_ASSERT(SUCCEEDED(hr));
        memcpy(m_blob->GetBufferPointer(), pData, sizeBytes);
    }
    LPVOID STDMETHODCALLTYPE GetBufferPointer(void) override
    {
        return m_blob->GetBufferPointer();
    }

    SIZE_T STDMETHODCALLTYPE GetBufferSize(void) override
    {
        return m_blob->GetBufferSize();
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID riid,
        void** ppvObject) override
    {
        return 0;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef( void) override { return 0; }

    virtual ULONG STDMETHODCALLTYPE Release( void) override { return 0; }
private:
    CComPtr<ID3DBlob> m_blob;
};

#if defined RCL_DXC

#define DXBC_FOURCC(ch0, ch1, ch2, ch3)                                        \
  ((UINT)(BYTE)(ch0) | ((UINT)(BYTE)(ch1) << 8) | ((UINT)(BYTE)(ch2) << 16) |  \
   ((UINT)(BYTE)(ch3) << 24))

R_INTERNAL UINT32 DXBC_DXIL = DXBC_FOURCC('D', 'X', 'I', 'L');          // == DFCC_DXIL

std::wstring getShaderProfile(ShaderType type)
{
    std::wstring model;
    switch (type) 
    {
        case ShaderType_Vertex: model = std::wstring(L"vs_"); break;
        case ShaderType_Pixel: model = std::wstring(L"ps_"); break;
        case ShaderType_Compute: model = std::wstring(L"cs_"); break; 
        case ShaderType_Geometry: model = std::wstring(L"gs_"); break;
        case ShaderType_Domain: model = std::wstring(L"ds_"); break;
        case ShaderType_Hull: model = std::wstring(L"hs_"); break;
        default: return L"unknown";
    }
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring version = converter.from_bytes(g_shaderModel);
    return model + version;
}


class DXCShaderBuilder : public ShaderBuilder 
{
public:
    DXCShaderBuilder(ShaderIntermediateCode imm)
        : ShaderBuilder(imm) { }

    ResultCode setUp() override
    {
        HRESULT hr = S_OK;
        hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library));
        
        if (FAILED(hr))
        {
            R_ERROR("DXC", "Failed to create library for parsing!");
            return RecluseResult_Failed;
        }

        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));

        if (FAILED(hr))
        {
            R_ERROR("DXC", "Failed to create dxc compiler!");
            return RecluseResult_Failed;
        }

        //hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
        //if (FAILED(hr))
        //{
        //    R_ERROR("DXC", "Failed to create dxc ultilities!");
        //    return RecluseResult_Failed;
        //}
        return RecluseResult_Ok;
    }

    ResultCode tearDown() override
    {
        if (m_compiler)
            m_compiler.Release();
        if (m_library)
            m_library.Release();
        return RecluseResult_Ok;
    }

    ResultCode onCompile
        (
            const std::vector<char>& srcCode, 
            std::vector<char>& byteCode,  
            const char* entryPoint,
            ShaderLanguage lang, 
            ShaderType shaderType, 
            const std::vector<PreprocessDefine>& defines
        ) override 
    {
        R_ASSERT(m_library != NULL);
        R_ASSERT(m_compiler != NULL);

        R_DEBUG("DXC", "Compiling shader...");
        
        struct NativeDefine
        {
            WCHAR* Name;
            WCHAR* Value;
        };        

        CComPtr<IDxcOperationResult> result;
        CComPtr<IDxcBlobEncoding> sourceBlob;
        std::vector<DxcDefine> dxcDefines;
        std::vector<NativeDefine> nativeDefines;
        nativeDefines.resize(defines.size());
        dxcDefines.resize(defines.size());
        for (U32 i = 0; i < nativeDefines.size(); ++i)
        {
            int count = MultiByteToWideChar(CP_UTF8, 0, defines[i].variable.c_str(), defines[i].variable.size(), nullptr, 0);
            nativeDefines[i].Name = new WCHAR[count + 1];
            MultiByteToWideChar(CP_UTF8, 0, defines[i].variable.c_str(), defines[i].variable.size(), const_cast<LPWSTR>(nativeDefines[i].Name), count);
            nativeDefines[i].Name[count] = L'\0';
            count = MultiByteToWideChar(CP_UTF8, 0, defines[i].value.c_str(), defines[i].value.size(), nullptr, 0);
            nativeDefines[i].Value = new WCHAR[count + 1];
            MultiByteToWideChar(CP_UTF8, 0, defines[i].value.c_str(), defines[i].value.size(), const_cast<LPWSTR>(nativeDefines[i].Value), count);
            nativeDefines[i].Value[count] = L'\0';
            dxcDefines[i].Name = nativeDefines[i].Name;
            dxcDefines[i].Value = nativeDefines[i].Value;
        }

        HRESULT hr                      = S_OK;
        std::wstring targetProfile      = getShaderProfile(shaderType);
        const wchar_t* arguments[16]    = { };
        U32 argCount                    = 0;

        // NOTE(): This doesn't work on older dxc compiler versions.
        //arguments[argCount++] = L"-Wignored-attributes";

        UINT32 srcSizeBytes = (UINT32)srcCode.size();
        hr = m_library->CreateBlobWithEncodingFromPinned(srcCode.data(), srcSizeBytes, CP_UTF8, &sourceBlob);
        
        if (FAILED(hr)) 
        {
            R_ERROR("DXC", "Failed to create a blob!!");
        }

        if (getIntermediateCode() == ShaderIntermediateCode_Spirv) 
        {
            arguments[argCount++] = L"-spirv";
        }

        int count = MultiByteToWideChar(CP_UTF8, 0, entryPoint, sizeof(entryPoint), nullptr, 0);
        WCHAR* wideEntryPoint = new WCHAR[count];
        MultiByteToWideChar(CP_UTF8, 0, entryPoint, sizeof(entryPoint), wideEntryPoint, count);

        hr = m_compiler->Compile
            (
                sourceBlob, 
                NULL, 
                wideEntryPoint, 
                targetProfile.c_str(), 
                arguments, argCount, 
                dxcDefines.empty() ? NULL : dxcDefines.data(), (UINT32)dxcDefines.size(), 
                NULL, (IDxcOperationResult**)&result
            );

        delete wideEntryPoint;
        
        CComPtr<IDxcBlobEncoding> errorBlob;
        result->GetErrorBuffer(&errorBlob);
        R_DEBUG("DXC", "\n%s", (const char*)errorBlob->GetBufferPointer());
        if (FAILED(hr)) 
        {
            return RecluseResult_Failed;
        }

        CComPtr<IDxcBlob> code;
        result->GetResult(&code);

        byteCode.resize(code->GetBufferSize());
        memcpy(byteCode.data(), code->GetBufferPointer(), code->GetBufferSize());

        for (U32 i = 0; i < nativeDefines.size(); ++i)
        {
            delete nativeDefines[i].Name;
            delete nativeDefines[i].Value;
        }

        return RecluseResult_Ok;
    }

    ResultCode reflect(ShaderReflection& reflectionOutput, const char* bytecode, U64 sizeBytes, ShaderLanguage lang) override
    {
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
        hr = containerReflection->FindFirstPartKind(hlsl::DFCC_DXIL, &shaderIndex);
        R_ASSERT(SUCCEEDED(hr));
        containerReflection->GetPartReflection(shaderIndex, __uuidof(ID3D12ShaderReflection), (void**)&shaderReflection);
        R_ASSERT(SUCCEEDED(hr));
        D3D12_SHADER_DESC shaderDesc = { };
        shaderReflection->GetDesc(&shaderDesc);
        reflectionOutput.metadata.numCbvs = shaderDesc.ConstantBuffers;
        U32 numResources = shaderDesc.BoundResources;
        for (U32 resourceIdx = 0; resourceIdx < numResources; ++resourceIdx)
        {
            D3D12_SHADER_INPUT_BIND_DESC shaderInputDesc = { };
            shaderReflection->GetResourceBindingDesc(resourceIdx, &shaderInputDesc);
            // For DXC, we store the register bind. (c<BindPoint>, c<BindPoint+1>, c<BindPoint+2>, c<BindPoint+3> ...)
            // This will store all bind points from HLSL -> D3D12.
            switch (shaderInputDesc.Type)
            {
                case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
                {
                    for (UINT bind = shaderInputDesc.BindPoint; bind < shaderInputDesc.BindCount; ++bind)
                    {
                        reflectionOutput.cbvs.push_back(static_cast<ShaderBind>(bind));
                    }
                    break;
                }
                case D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED:
                case D3D_SHADER_INPUT_TYPE::D3D_SIT_BYTEADDRESS:
                case D3D_SHADER_INPUT_TYPE::D3D_SIT_TBUFFER:
                case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
                {
                    for (UINT bind = shaderInputDesc.BindPoint; bind < shaderInputDesc.BindCount; ++bind)
                    {
                        reflectionOutput.srvs.push_back(static_cast<ShaderBind>(bind));                    
                        reflectionOutput.metadata.numSrvs += 1;
                    }
                    break;
                }
                case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
                {
                    for (UINT bind = shaderInputDesc.BindPoint; bind < shaderInputDesc.BindCount; ++bind)
                    {
                        reflectionOutput.samplers.push_back(static_cast<ShaderBind>(bind));                    
                        reflectionOutput.metadata.numSamplers += 1;
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
                    for (UINT bind = shaderInputDesc.BindPoint; bind < shaderInputDesc.BindCount; ++bind)
                    {
                        reflectionOutput.uavs.push_back(static_cast<ShaderBind>(bind));                    
                        reflectionOutput.metadata.numUavs += 1;
                    }
                    break;
                }
            }
        }
        return RecluseResult_Ok;
    }

private:
    CComPtr<IDxcCompiler> m_compiler;
    CComPtr<IDxcLibrary> m_library;
    //CComPtr<IDxcUtils> m_utils;
};
#endif

ShaderBuilder* createDxcShaderBuilder(ShaderIntermediateCode imm)
{
#if defined RCL_DXC
    return new DXCShaderBuilder(imm);
#else
    R_ERROR("DXC", "DXC was not enabled for compilation!");
    return nullptr;
#endif
}
} // Pipeline
} // Recluse