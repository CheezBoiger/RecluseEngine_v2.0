//
#include "ShaderBuilder.hpp"
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
    switch (type) {
        case SHADER_TYPE_VERTEX: return L"vs_6_0";
        case SHADER_TYPE_PIXEL: return L"ps_6_0";
        case SHADER_TYPE_COMPUTE: return L"cs_6_0"; 
        default: return L"unknown";
    }
}


class DXCShaderBuilder : public ShaderBuilder {
public:
    DXCShaderBuilder(ShaderType shaderType, ShaderIntermediateCode imm)
        : ShaderBuilder(shaderType, imm) { }

    ErrType compile(const std::vector<char>& srcCode, std::vector<char>& byteCode, ShaderLang lang) override 
    {
        R_DEBUG("DXC", "Compiling shader...");

        HRESULT hr              = S_OK;
        wchar_t* targetProfile  = getShaderProfile(getShaderType());
        const wchar_t* arguments[16]  = { };
        U32 argCount            = 0;

        CComPtr<IDxcCompiler> compiler;
        CComPtr<IDxcOperationResult> result;
        CComPtr<IDxcBlobEncoding> sourceBlob;
        CComPtr<IDxcLibrary> library;
        
        hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

        if (FAILED(hr)) {
        
            R_ERR("DXC", "Failed to create library for parsing!");

            return REC_RESULT_FAILED;
        
        }

        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        
        if (FAILED(hr)) {

            R_ERR("DXC", "Failed to create dxc compiler!");        
            
            return REC_RESULT_FAILED;

        }

        UINT32 srcSizeBytes = (UINT32)srcCode.size();
        hr = library->CreateBlobWithEncodingFromPinned(srcCode.data(), srcSizeBytes, CP_UTF8, &sourceBlob);
        
        if (FAILED(hr)) {
            R_ERR("DXC", "Failed to create a blob!!");
        }

        if (getIntermediateCode() == INTERMEDIATE_SPIRV) {
            arguments[argCount++] = L"-spirv";
        }
        
        hr = compiler->Compile(
            sourceBlob, 
            NULL, 
            L"main", 
            targetProfile, 
            arguments, argCount, 
            NULL, 0, 
            NULL, &result);

        if (FAILED(hr)) {

            CComPtr<IDxcBlobEncoding> errorBlob;
            hr = result->GetErrorBuffer(&errorBlob);

            if (SUCCEEDED(hr) && errorBlob) {

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
};
#endif

ShaderBuilder* createDxcShaderBuilder(ShaderType shaderType, ShaderIntermediateCode imm)
{
#if defined RCL_DXC
    return new DXCShaderBuilder(shaderType, imm);
#else
    R_ERR("DXC", "DXC was not enabled for compilation!");
    return nullptr;
#endif
}
} // Recluse