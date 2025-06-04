//
#include "Recluse/Messaging.hpp"
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Recluse/Utility.hpp"
#include "Win32/Win32Common.hpp"

#include "Recluse/Messaging.hpp"

#include <codecvt>
#include <locale>
#include <map>

#if defined RCL_DXC 
#include <atlbase.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#endif

R_DECLARE_GLOBAL_STRING(g_shaderModel, "6_5", "DXC.ShaderModel");
R_DECLARE_GLOBAL_STRING(g_meshShaderModel, "6_5", "DXC.MeshShaderTargetModel");
R_DECLARE_GLOBAL_STRING(g_ampShaderModel, "6_5", "DXC.AmpShaderTargetModel");

namespace Recluse {
namespace Pipeline {


// Direct blob. 
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

std::wstring getShaderProfile(ShaderType type)
{
    std::wstring model;
    switch (type) 
    {
        case ShaderType_Vertex:         model = std::wstring(L"vs_"); break;
        case ShaderType_Pixel:          model = std::wstring(L"ps_"); break;
        case ShaderType_Compute:        model = std::wstring(L"cs_"); break;
        case ShaderType_Geometry:       model = std::wstring(L"gs_"); break;
        case ShaderType_Domain:         model = std::wstring(L"ds_"); break;
        case ShaderType_Hull:           model = std::wstring(L"hs_"); break;
        case ShaderType_Mesh:           model = std::wstring(L"ms_"); break;
        case ShaderType_Amplification:  model = std::wstring(L"as_"); break; 
        default: return L"unknown";
    }
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring version = converter.from_bytes(g_shaderModel);

    switch (type)
    {
        case ShaderType_Mesh:
            version = converter.from_bytes(g_meshShaderModel);
            break;
        case ShaderType_Amplification:
            version = converter.from_bytes(g_ampShaderModel);
            break;
        default:
            break;
    }

    return model + version;
}


class DXCShaderBuilder : public ShaderBuilder 
{
public:
    DXCShaderBuilder()
        : ShaderBuilder() 
    {
        m_optimizationMap = { 
            { Config::Default, L"-O3" },
            { Config::Disable, L"-O0" }
        }; 
    }

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

    // On compile.
    ResultCode onCompile
        (
            const std::vector<char>& srcCode, 
            std::vector<char>& byteCode,  
            const char* entryPoint,
            ShaderLanguage lang, 
            ShaderType shaderType, 
            ShaderIntermediateCode intermediateCode,
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

        if (intermediateCode == ShaderIntermediateCode_Spirv) 
        {
            arguments[argCount++] = L"-spirv";
        }

        // Optimization settings
        arguments[argCount++] = m_optimizationMap[getOptimizationOption()];

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

private:
    CComPtr<IDxcCompiler> m_compiler;
    CComPtr<IDxcLibrary> m_library;
    //CComPtr<IDxcUtils> m_utils;
    std::map<Config::OptimizationOption, const wchar_t*> m_optimizationMap;
};
#endif

ShaderBuilder* createDxcShaderBuilder()
{
#if defined RCL_DXC
    return new DXCShaderBuilder();
#else
    R_ERROR("DXC", "DXC was not enabled for compilation!");
    return nullptr;
#endif
}
} // Pipeline
} // Recluse