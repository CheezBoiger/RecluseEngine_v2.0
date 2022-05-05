//
#include "Recluse/Graphics/ShaderBuilder.hpp"
#include "Win32/Win32Common.hpp"
#include "Recluse/Messaging.hpp"

#if defined RCL_DXC 
#include <atlbase.h>
#include <dxcapi.h>
#endif
namespace Recluse {

#if defined RCL_DXC

wchar_t* getShaderProfile(ShaderType type)
{
    switch (type) 
    {
        case SHADER_TYPE_VERTEX: return L"vs_6_0";
        case SHADER_TYPE_PIXEL: return L"ps_6_0";
        case SHADER_TYPE_COMPUTE: return L"cs_6_0"; 
        default: return L"unknown";
    }
}


class DXCShaderBuilder : public ShaderBuilder 
{
public:
    DXCShaderBuilder(ShaderIntermediateCode imm)
        : ShaderBuilder(imm) { }

    ErrType setUp() override
    {
        HRESULT hr = S_OK;
        hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library));
        
        if (FAILED(hr))
        {
            R_ERR("DXC", "Failed to create library for parsing!");
            return REC_RESULT_FAILED;
        }

        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));

        if (FAILED(hr))
        {
            R_ERR("DXC", "Failed to create dxc compiler!");
            return REC_RESULT_FAILED;
        }

        return REC_RESULT_OK;
    }

    ErrType tearDown() override
    {
        if (m_compiler)
            m_compiler.Release();
        if (m_library)
            m_library.Release();
        return REC_RESULT_OK;
    }

    ErrType onCompile(const std::vector<char>& srcCode, std::vector<char>& byteCode, 
        ShaderLang lang, ShaderType shaderType) override 
    {
        R_ASSERT(m_library != NULL);
        R_ASSERT(m_compiler != NULL);

        R_DEBUG("DXC", "Compiling shader...");
        
        CComPtr<IDxcOperationResult> result;
        CComPtr<IDxcBlobEncoding> sourceBlob;

        HRESULT hr                      = S_OK;
        wchar_t* targetProfile          = getShaderProfile(shaderType);
        const wchar_t* arguments[16]    = { };
        U32 argCount                    = 0;

        UINT32 srcSizeBytes = (UINT32)srcCode.size();
        hr = m_library->CreateBlobWithEncodingFromPinned(srcCode.data(), srcSizeBytes, CP_UTF8, &sourceBlob);
        
        if (FAILED(hr)) 
        {
            R_ERR("DXC", "Failed to create a blob!!");
        }

        if (getIntermediateCode() == INTERMEDIATE_SPIRV) 
        {
            arguments[argCount++] = L"-spirv";
        }
        
        hr = m_compiler->Compile
            (
                sourceBlob, 
                NULL, 
                L"main", 
                targetProfile, 
                arguments, argCount, 
                NULL, 0, 
                NULL, &result
            );

        if (FAILED(hr)) 
        {
            CComPtr<IDxcBlobEncoding> errorBlob;
            hr = result->GetErrorBuffer(&errorBlob);

            if (SUCCEEDED(hr) && errorBlob) 
            {
                R_ERR("DXC", "Errors found: \n%s", (const char*)errorBlob->GetBufferPointer());   
            }   
        
            return REC_RESULT_FAILED;
        }

        CComPtr<IDxcBlob> code;
        result->GetResult(&code);

        byteCode.resize(code->GetBufferSize());
        memcpy(byteCode.data(), code->GetBufferPointer(), code->GetBufferSize());

        return REC_RESULT_OK;
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
    R_ERR("DXC", "DXC was not enabled for compilation!");
    return nullptr;
#endif
}
} // Recluse