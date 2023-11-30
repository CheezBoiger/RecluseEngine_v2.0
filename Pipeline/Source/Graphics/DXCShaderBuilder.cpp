//
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Win32/Win32Common.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Utility.hpp"

#include <locale>
#include <codecvt>

#if defined RCL_DXC 
#include <atlbase.h>
#include <dxcapi.h>
#endif

R_DECLARE_GLOBAL_STRING(g_shaderModel, "6_0", "DXC.ShaderModel");

namespace Recluse {
namespace Pipeline {

#if defined RCL_DXC

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

    ResultCode onCompile(const std::vector<char>& srcCode, std::vector<char>& byteCode,  const char* entryPoint,
        ShaderLang lang, ShaderType shaderType, const std::vector<PreprocessDefine>& defines) override 
    {
        R_ASSERT(m_library != NULL);
        R_ASSERT(m_compiler != NULL);

        R_DEBUG("DXC", "Compiling shader...");
        
        CComPtr<IDxcOperationResult> result;
        CComPtr<IDxcBlobEncoding> sourceBlob;

        HRESULT hr                      = S_OK;
        std::wstring targetProfile      = getShaderProfile(shaderType);
        const wchar_t* arguments[16]    = { };
        U32 argCount                    = 0;

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
                NULL, 0, 
                NULL, &result
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

        return RecluseResult_Ok;
    }

private:
    CComPtr<IDxcCompiler> m_compiler;
    CComPtr<IDxcLibrary> m_library;
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