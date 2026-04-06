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
R_DECLARE_GLOBAL_BOOLEAN(g_meshShaderSpirvUseNV, false, "DXC.SpirvUseNVExtension");
R_DECLARE_GLOBAL_BOOLEAN(g_useLegacyResourceReservation, true, "DXC.LegacyResourceReservation");

namespace Recluse {
namespace Pipeline {

#if defined RCL_DXC

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
            { Config::Disable, L"-Od" }
        }; 
    }

    ResultCode setUp() override
    {
        HRESULT hr = S_OK;
        hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
        
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

        hr = m_utils->CreateDefaultIncludeHandler(&m_includeHandler);
        if (FAILED(hr))
        {
            R_ERROR("DXC", "Failed to create dxc include handler!");
            return RecluseResult_Failed;
        }

        return RecluseResult_Ok;
    }

    ResultCode tearDown() override
    {
        if (m_compiler)
            m_compiler.Release();
        if (m_includeHandler)
            m_includeHandler.Release();
        if (m_utils)
            m_utils.Release();
        return RecluseResult_Ok;
    }

    // On compile.
    ResultCode onCompile
        (
            const std::vector<char>& srcCode, 
            std::vector<char>& byteCode,  
            const char* entryPoint,
            const ShaderBuilder::Config& config,
            ShaderDebug* shaderDebugOut,
            const std::vector<PreprocessDefine>& defines
        ) override 
    {
        R_ASSERT(m_utils != NULL);
        R_ASSERT(m_compiler != NULL);

        R_DEBUG("DXC", "Compiling shader...");

        struct NativeDefine
        {
            WCHAR* Name;
            WCHAR* Value;
        };        

        HRESULT hr = S_OK;
        std::wstring targetProfile = getShaderProfile(config.shaderType);

        std::vector<std::wstring> argStrings;

        // Optimization settings
        argStrings.push_back(m_optimizationMap[config.option]);

        int count = MultiByteToWideChar(CP_UTF8, 0, entryPoint, strlen(entryPoint), nullptr, 0);
        WCHAR* wideEntryPoint = new WCHAR[count + 1];
        MultiByteToWideChar(CP_UTF8, 0, entryPoint, count, wideEntryPoint, count);

        wideEntryPoint[count] = L'\0';

        argStrings.push_back(L"-E");
        argStrings.push_back(wideEntryPoint);

        delete[] wideEntryPoint;

        argStrings.push_back(L"-T");
        argStrings.push_back(targetProfile);

        for (U32 i = 0; i < defines.size(); ++i)
        {
            NativeDefine nativeDefine;
            int count = MultiByteToWideChar(CP_UTF8, 0, defines[i].variable.c_str(), defines[i].variable.size(), nullptr, 0);

            nativeDefine.Name = new WCHAR[count + 1];

            MultiByteToWideChar(CP_UTF8, 0, defines[i].variable.c_str(), defines[i].variable.size(), const_cast<LPWSTR>(nativeDefine.Name), count);
            nativeDefine.Name[count] = L'\0';
            count = MultiByteToWideChar(CP_UTF8, 0, defines[i].value.c_str(), defines[i].value.size(), nullptr, 0);

            nativeDefine.Value = new WCHAR[count + 1];

            MultiByteToWideChar(CP_UTF8, 0, defines[i].value.c_str(), defines[i].value.size(), const_cast<LPWSTR>(nativeDefine.Value), count);
            nativeDefine.Value[count] = L'\0';

            argStrings.push_back(L"-D");
            argStrings.push_back(std::wstring(nativeDefine.Name) + L"=" + std::wstring(nativeDefine.Value));

            delete[] nativeDefine.Name;
            delete[] nativeDefine.Value;
        }

        // NOTE(): This doesn't work on older dxc compiler versions.
        //arguments[argCount++] = L"-Wignored-attributes";

        if (config.intermediateCode == ShaderIntermediateCode_Spirv) 
        {
            argStrings.push_back(L"-spirv");
            if (config.shaderType == ShaderType_Mesh || config.shaderType == ShaderType_Amplification)
            {
                if (!g_meshShaderSpirvUseNV)
                {
                    // SPIRV 1.4 is required to use SPV_EXT_mesh_shader.
                    argStrings.push_back(L"-fspv-target-env=vulkan1.1spirv1.4");
                    argStrings.push_back(L"-fspv-extension=SPV_EXT_mesh_shader");
                }
            }
        }

        if (g_useLegacyResourceReservation)
        {
            // Maintain legacy resource binding, to prevent stripping if the resource is unused.
            argStrings.push_back(L"-flegacy-resource-reservation");
        }

        if (config.dumpSymbols)
        {
            argStrings.push_back(L"-Zi");
            if (config.intermediateCode == ShaderIntermediateCode_Spirv)
            {
                argStrings.push_back(L"-fspv-debug=vulkan-with-source");
            }
            else 
            {
                if (config.stripDebugInfo)
                {
                    argStrings.push_back(L"-Qstrip_debug");
                }
                argStrings.push_back(L"-Fd");
                argStrings.push_back(L".\\");
            }
        }
        
        DxcBuffer sourceBuffer = { };
        sourceBuffer.Ptr = srcCode.data();
        sourceBuffer.Size = srcCode.size();
        sourceBuffer.Encoding = DXC_CP_UTF8;

        std::vector<LPCWSTR> arguments;
        for (const auto& ws : argStrings)
        {
            arguments.push_back(ws.c_str());
        }

        CComPtr<IDxcResult> result;

        hr = m_compiler->Compile
            (
                &sourceBuffer, 
                arguments.data(),
                arguments.size(),
                m_includeHandler,
                __uuidof(IDxcResult),
                (void**)&result
            );

        CComPtr<IDxcBlobEncoding> errorBlob;
        result->GetErrorBuffer(&errorBlob);
        R_DEBUG("DXC", "\n%s", (const char*)errorBlob->GetBufferPointer());
        if (FAILED(hr)) 
        {
            return RecluseResult_Failed;
        }

        CComPtr<IDxcBlob> code;
        CComPtr<IDxcBlobUtf16> pShaderName;

        result->GetOutput(DXC_OUT_OBJECT, __uuidof(IDxcBlob), (void**)&code, &pShaderName);

        if (code)
        {
            byteCode.resize(code->GetBufferSize());
            memcpy(byteCode.data(), code->GetBufferPointer(), code->GetBufferSize());
            return RecluseResult_Ok;
        }

        if (result->HasOutput(DXC_OUT_PDB))
        {
            CComPtr<IDxcBlob> pdbBlob;
            CComPtr<IDxcBlobUtf16> pdbBlobName;
            result->GetOutput(DXC_OUT_PDB, __uuidof(IDxcBlob), (void**)&pdbBlob, &pdbBlobName);
            if (pdbBlob)
            {
                //
            }
        }
        return RecluseResult_Failed;
    }

private:
    CComPtr<IDxcCompiler3> m_compiler;
    CComPtr<IDxcUtils> m_utils;
    CComPtr<IDxcIncludeHandler> m_includeHandler;

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