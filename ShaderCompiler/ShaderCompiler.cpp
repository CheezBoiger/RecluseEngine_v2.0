//
#include "ShaderCompiler.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/ShaderBuilder.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include "json.hpp"

#include <string>
#include <sstream>
#include <map>
#include <unordered_map>

using json = nlohmann::json;

namespace Recluse {


ShaderLang kDefaultShaderLanguage = ShaderLang_Hlsl;

// Serialize our enum types with the following for json configs.
NLOHMANN_JSON_SERIALIZE_ENUM(Recluse::ShaderStage,
    {
        { Recluse::ShaderStage_Vertex,          "vertex" },
        { Recluse::ShaderStage_Pixel,           "pixel" },
        { Recluse::ShaderStage_Fragment,        "frag" },
        { Recluse::ShaderStage_Hull,            "hull" },
        { Recluse::ShaderStage_Domain,          "domain" },
        { Recluse::ShaderStage_Compute,         "compute" },
        { Recluse::ShaderStage_TessellationControl,    "tessc"},
        { Recluse::ShaderStage_TessellationEvaluation,       "tesse" },
        { Recluse::ShaderStage_Geometry,        "geometry"},
        { Recluse::ShaderStage_RayAnyHit,      "anyhit" },
        { Recluse::ShaderStage_RayClosestHit,  "closesthit" },
        { Recluse::ShaderStage_RayGeneration,         "raygen" },
        { Recluse::ShaderStage_RayIntersect,   "intersect" },
        { Recluse::ShaderStage_RayMiss,        "raymiss" },
        { Recluse::ShaderStage_Amplification,   "amp" },
        { Recluse::ShaderStage_Mesh,            "mesh" }
    }
)


NLOHMANN_JSON_SERIALIZE_ENUM(Recluse::ShaderLang,
    {
        { Recluse::ShaderLang_Glsl, "glsl" },
        { Recluse::ShaderLang_Hlsl, "hlsl" }
    }
)


#define SCENE_BUFFER_DECLARE_STR "RECLUSE_DECLARE_SCENE_BUFFER"
#define SCENE_BUFFER_PARAM_STR "RECLUSE_SCENE_PARAMETER"
#define SCENE_BUFFER_END_STR    "RECLUSE_END_SCENE_BUFFER"

#define SCENE_BUFFER_INCLUDE "RecluseSceneBuffer.h"

#define MAP_SHADER_KEYWORD(keyword, glslKeyword, hlslKeyword) { keyword, { { ShaderLang_Glsl, glslKeyword }, { ShaderLang_Hlsl, hlslKeyword } } }

static std::unordered_map<std::string, std::map<ShaderLang, std::string>> kShaderKeywordMap = 
{
    MAP_SHADER_KEYWORD("struct",    "struct",   "struct"),
    MAP_SHADER_KEYWORD("Matrix44",  "mat4",     "float4x4"),
    MAP_SHADER_KEYWORD("Matrix33",  "mat3",     "float3x3"),
    MAP_SHADER_KEYWORD("F32",       "float",    "float"),
    MAP_SHADER_KEYWORD("Float2",    "vec2",     "float2"),
    MAP_SHADER_KEYWORD("Float3",    "vec3",     "float3"),
    MAP_SHADER_KEYWORD("Float4",    "vec4",     "float4"),
    MAP_SHADER_KEYWORD("I32",       "int",      "int"),
    MAP_SHADER_KEYWORD("U32",       "uint",     "uint")
};

#define GET(key, lang) kShaderKeywordMap[key][lang]


struct ShaderMetaData 
{
    std::string                     configs;
    std::string                     relativefilePath;
    ShaderType                      shaderType;
    const char*                     entryPoint;
    std::map<std::string, void*>    preprocess;
};


struct CompilerState 
{
    std::string name;
    ShaderLang  language;
    std::string outputExt;
    std::string version;
    B32         appendExt;
    
    struct 
    {
        // Global extension of our shaders.
        // usually defined as ".*shaderTypeExt.*ext"
        std::string                         ext;
        std::map<ShaderType, std::string>   shaderTypeExt;
    } input;

    CompilerState() {
        input.ext = "";
    }
};


static struct 
{
    std::string                 sourcePath;
    std::vector<CompilerState>  compilers;
    std::vector<ShaderMetaData> shadersToCompile;
    CompilerState*              pCompilerState;
} gConfigs;


void replaceLine(std::string& in, size_t currentPos, const std::string& from, const std::string& to)
{
    in.replace(currentPos, from.size(), to);
}


static std::string generateShaderSceneView(ShaderLang lang)
{
    ErrType result = RecluseResult_Ok;
    // Scene view buffer string to be passed.
    std::string sceneViewBufferStr  = "#ifndef _RECLUSE_SCENE_BUFFER_H_\n#define _RECLUSE_SCENE_BUFFER_H_\n";
    // Start at this file and navigate to the Engine scene view file.
    std::string kShaderCompilerFile = Filesystem::getDirectoryFromPath(__FILE__);
    // Get the file we need.
    // TODO(): We may want to specify this as a path outside of the ShaderCompiler. In short, we want to 
    //          have our path defined in the json file.
    std::string pathToSceneViewFile = kShaderCompilerFile + "/../Engine/Include/Recluse/Renderer/SceneView.hpp";
    
    FileBufferData data             = { };

    result = File::readFrom(&data, pathToSceneViewFile);

    if (result != RecluseResult_Ok) 
    {    
        R_ERR("ShaderCompiler", "Failed to read from file: %s", pathToSceneViewFile.c_str());
        return "";
    }

    // Read the stream.
    std::istringstream iss(data.data());
    std::string line;
    B32 insideSceneViewStruct = false;

    // Start looking for our scene view declaration struct!
    while (std::getline(iss, line)) 
    {    
        R_DEBUG("ShaderCompiler", "%s", line.c_str());

        if 
            (
                line.find(SCENE_BUFFER_PARAM_STR) != std::string::npos 
                && insideSceneViewStruct
            ) 
        {
            // Parameter parsing.
            size_t start    = line.find_first_of('(') + 1;
            size_t end      = line.find_first_of(',');
            size_t sz       = end - start;

            if (sz == 0) 
            {
                R_ERR("ShaderCompiler", "Variable syntax declared, but no name found!");
            }

            std::string varName = line.substr(start, sz);
            start               = end + 1;
            end                 = line.find_first_of(')');
            sz                  = end - start;

            if (sz == 0) 
            {
                R_ERR("ShaderCompiler", "Data type not written!");
            }

            std::string varDataType = line.substr(start, sz);

            varDataType.erase
                            (
                                std::remove_if(varDataType.begin(), varDataType.end(), std::isspace),
                                varDataType.end()
                            );

            {
                U64 ns = varDataType.find_last_of(':');
                if (ns != std::string::npos)
                {
                    U64 diff = varDataType.size() - diff;
                    varDataType = varDataType.substr(ns, diff);
                }
            }
            
            sceneViewBufferStr += "    " + GET(varDataType, lang) + " " + varName + ";\n";
        } 
        else if 
            (
                line.find(SCENE_BUFFER_DECLARE_STR) != std::string::npos
                // We also need to make sure we aren't also parsing the macro declaration...
                && line.find("#define") == std::string::npos
            ) 
        {

            insideSceneViewStruct   = true;
            std::string structName  = "";
            size_t start            = line.find_first_of('(') + 1;
            size_t end              = line.find_first_of(')');
            size_t sz               = end - start;

            if (start == std::string::npos || end == std::string::npos) 
            {
                R_ERR("ShaderCompiler", "No struct name provided for scene buffer!");
            }

            structName = line.substr(start, sz);

            sceneViewBufferStr   += GET("struct", lang) + " " + structName + " {\n";
            R_VERBOSE("ShaderCompiler", "Found the scene view declarer!");
        } 
        else if 
            (
                line.find(SCENE_BUFFER_END_STR) != std::string::npos
                // Check we aren't parsing the macro declaration...
                && line.find("#define") == std::string::npos
            ) 
        {
            insideSceneViewStruct = false;
            R_VERBOSE("ShaderCompiler", "Ending scene view declaration struct...");
            sceneViewBufferStr += " }; \n";

        }
    }

    sceneViewBufferStr += "#endif\n";

    return sceneViewBufferStr;
}


ShaderBuilder* createBuilder(Allocator* scratch, ShaderLang lang, ShaderIntermediateCode intermediateCode)
{
    ShaderBuilder* pBuilder = nullptr;
    
    switch (lang)
    {
        case ShaderLang_Hlsl:
        {
            pBuilder = createDxcShaderBuilder(intermediateCode);
            break;
        }
        case ShaderLang_Glsl:
        {
            pBuilder = createGlslangShaderBuilder(intermediateCode);
            break;
        }
        default:
        {
            break;
        }
    }

    return pBuilder;
}

ErrType compileShaders(ShaderLang lang)
{
    R_ASSERT(gConfigs.pCompilerState != NULL);

    std::string sceneViewBufferStructStr    = generateShaderSceneView(lang);
    std::string sourcePath                  = gConfigs.sourcePath;

    R_DEBUG("ShaderCompiler", "Result:\n%s", sceneViewBufferStructStr.c_str());

    // Set up the shader builder. We possibly want to be able to switch this out 
    // if need be.
    ScratchMemory scratchPool = { 1024 };
    LinearAllocator linearAllocation;
    linearAllocation.initialize(scratchPool.getBaseAddress(), scratchPool.getTotalSizeBytes());

    std::map<ShaderLang, ShaderBuilder*> shaderBuilders;

    ShaderBuilder* pBuilder = createBuilder(&linearAllocation, lang, ShaderIntermediateCode_Spirv);

    R_ASSERT(pBuilder != NULL);

    pBuilder->setUp();

    for 
        (
            auto& shaderMetadata = gConfigs.shadersToCompile.begin(); 
            shaderMetadata != gConfigs.shadersToCompile.end(); 
            ++shaderMetadata
        ) 
    {
        std::string sourceFilePath = sourcePath + "/" + shaderMetadata->relativefilePath;
        Shader* pShader = Shader::create();
        FileBufferData buffer = { };

        sourceFilePath = sourceFilePath + "." + 
            gConfigs.pCompilerState->input.shaderTypeExt[shaderMetadata->shaderType];

        if (!gConfigs.pCompilerState->input.ext.empty()) 
        {
            sourceFilePath = sourceFilePath + "." + gConfigs.pCompilerState->input.ext;
        }

        ErrType result = File::readFrom(&buffer, sourceFilePath);

        if (result != RecluseResult_Ok) 
        {
            R_ERR("ShaderCompiler", "Failed to read shader %s...", shaderMetadata->relativefilePath.c_str());
            Shader::destroy(pShader);
            continue;
        } 
        else 
        {
                // Read the file buffer, and check for the scene buffer header include.
                size_t prevPos              = 0;
                std::string str             = buffer.data();

                // Remove the access allocation from the std library string. Still need to figure out why it does that?
                str = str.substr(0, buffer.size());
                std::istringstream iss(str);
                std::string line;

                while (std::getline(iss, line)) 
                {
                    size_t currentPos = iss.tellg();
                    size_t pos = line.find(SCENE_BUFFER_INCLUDE); 
                    if (pos != std::string::npos) 
                    {
                        // We found the include, now replace with the struct header.
                        replaceLine(str, prevPos, line, sceneViewBufferStructStr);
                        // Update the stringstream.
                        iss.str(str);
                        iss.seekg(prevPos);
                    } 
                    else if (line.find("#include") == 0) 
                    {
                        //  include the file in, because we need to find nested files.
                        size_t subIDx = line.find_first_of("\"") + 1;
                        std::string includeFilePath = line.substr(subIDx, line.size() - subIDx);
                        std::string includeSource = "";
                        subIDx = includeFilePath.find_first_of("\"");
                        includeFilePath = includeFilePath.substr(0, subIDx);
                        result = File::readFrom(&buffer, gConfigs.sourcePath + "/" + includeFilePath);
                        includeSource = buffer.data();
                        includeSource = includeSource.substr(0, buffer.size());
                        replaceLine(str, prevPos, line, includeSource);
                        // update the stringstream.
                        iss.str(str);
                        iss.seekg(prevPos);
                    } 
                    else 
                    {
                        prevPos = currentPos;
                    }
                }    
                
                R_DEBUG("ShaderCompiler", "%s", str.c_str());
                result = pBuilder->compile
                                        (
                                            pShader, 
                                            shaderMetadata->entryPoint,
                                            str.c_str(), 
                                            str.size(), 
                                            lang, 
                                            shaderMetadata->shaderType
                                        );

                if (result == RecluseResult_Ok) 
                {
                    //pShader->saveToFile()
                } 
                else 
                {
                    R_ERR
                        (
                            "ShaderCompiler", 
                            "Failed to compile shader! %s", 
                            shaderMetadata->relativefilePath.c_str()
                        );
                }
        }

        // Destroy the shader when finished.
        Shader::destroy(pShader);
    }

    pBuilder->tearDown();
    freeShaderBuilder(pBuilder);
    linearAllocation.cleanUp();

    return RecluseResult_Ok;
}


ErrType addShaderToCompile(const std::string& filePath, const char* entryPoint, const std::string& config, ShaderType shaderType)
{
    ShaderMetaData metadata     = { };
    metadata.configs            = config;
    metadata.relativefilePath   = filePath;
    metadata.shaderType         = shaderType;
    metadata.entryPoint         = entryPoint;

    gConfigs.shadersToCompile.push_back(metadata);

    return RecluseResult_Ok;
}


ErrType setConfigs(const std::string& configPath, U32 compilerIndex)
{
    json jfile;
    {
        Recluse::FileBufferData dat;
        Recluse::ErrType result = Recluse::File::readFrom(&dat, configPath);

        if (result != Recluse::RecluseResult_Ok) 
        {
            R_ERR("ShaderCompiler", "Failed to find config file!");
            return result;
        }

        std::istringstream iss(dat.data());

        try 
        {
            iss >> jfile;
        } 
        catch (std::exception& e) 
        {
            R_ERR("ShaderCompiler", "%s", e.what());
            return RecluseResult_Failed;
        }
    }

    if (jfile.find("compiler") != jfile.end()) 
    {
        auto compilers = jfile["compiler"];

        for (int i = 0; i < compilers.size(); ++i) 
        {
            CompilerState compilerState = { };
            auto compiler               = compilers[i];
            ShaderLang lang             = compiler["language"].get<ShaderLang>();
            std::string name            = compiler["name"].get<std::string>();
            std::string ext             = "";
            B32 appendExt               = false;
            compilerState.name          = name;
            compilerState.language      = lang;
            const char* langName        = "";

            if      (lang == ShaderLang_Glsl)  langName = "glsl";
            else if (lang == ShaderLang_Hlsl)  langName = "hlsl";

            if (compiler.find("output") != compiler.end()) 
            {
                auto output = compiler["output"];
                ext         = output["ext"].get<std::string>();
                appendExt   = output["append_shader_ext"].get<bool>();

                compilerState.appendExt = appendExt;
                compilerState.outputExt = ext;
            } 
            else 
            {
                R_ERR("ShaderCompiler", "Could not find the proper output parameters for this compiler info!");
            }

            if (compiler.find("input") != compiler.end()) 
            {
                auto input              = compiler["input"];
                compilerState.input.ext = input["ext"].get<std::string>();     
            }

            R_INFO
                (
                    "ShaderCompiler", 
                    "Name: %s, Language: %s, ext: %s, append ext: %s", 
                    name.c_str(), 
                    langName, ext.c_str(), 
                    (appendExt ? "true" : "false")
                );

            gConfigs.compilers.push_back(compilerState);
        }
    }

    // For now we will give all compilers the same shader type ext.
    if (jfile.find("configs") != jfile.end()) 
    {
        auto configs    = jfile["configs"];
        auto shaders    = configs["shaders"];
        auto graphics   = shaders["graphics"];
        auto general    = shaders["general"];

        for (auto& compiler : gConfigs.compilers) 
        {
            auto shader = graphics["pixel"];
            compiler.input.shaderTypeExt[ShaderType_Pixel]     = shader["ext"].get<std::string>();
            shader = graphics["vertex"];
            compiler.input.shaderTypeExt[ShaderType_Vertex]    = shader["ext"].get<std::string>();
            shader = graphics["hull"];
            compiler.input.shaderTypeExt[ShaderType_Hull]      = shader["ext"].get<std::string>();
            shader = graphics["domain"];
            compiler.input.shaderTypeExt[ShaderType_Domain]    = shader["ext"].get<std::string>();
            shader = graphics["geometry"];
            compiler.input.shaderTypeExt[ShaderType_Geometry]  = shader["ext"].get<std::string>();
            shader = general["compute"];
            compiler.input.shaderTypeExt[ShaderType_Compute]   = shader["ext"].get<std::string>();
        }
    }

    if (compilerIndex < gConfigs.compilers.size()) 
    {
        gConfigs.pCompilerState = &gConfigs.compilers[compilerIndex];
    } 
    else 
    {
        R_ERR("ShaderCompiler", "No compilers defined in config file!!");
    }

    return RecluseResult_Ok;
}


ErrType setShaderFiles(const std::string& shadersPath)
{
    json jfile;
    ErrType result = RecluseResult_Ok;
    {
        FileBufferData bufferData = { };
        result = File::readFrom(&bufferData, shadersPath);

        if (result != RecluseResult_Ok) 
        {
            R_ERR("ShaderCompiler", "Failed to open shaders path!");
            return result;        
        }

        std::istringstream iss(bufferData.data());
        try 
        {
            iss >> jfile;
        } 
        catch (const std::exception& e) 
        {
            R_ERR("ShaderCompiler", "%s", e.what());
            return RecluseResult_Failed;
        }
    }

    if (jfile.find("global") != jfile.end()) 
    {
        auto global             = jfile["global"];
        std::string sourcePath  = global["source_path"].get<std::string>();
        gConfigs.sourcePath     = sourcePath;
    }

    if (jfile.find("shaders") != jfile.end()) 
    {
        auto shaders = jfile["shaders"];
        if (shaders.is_array()) 
        {
            size_t sz = shaders.size();
            for (U32 i = 0; i < sz; ++i) 
            {
                std::string name                = shaders[i]["name"].get<std::string>();
                ShaderType shaderType           = shaders[i]["type"].get<ShaderType>();
                ShaderMetaData shaderMetadata   = { };
                shaderMetadata.relativefilePath = name;
                shaderMetadata.shaderType       = shaderType;

                gConfigs.shadersToCompile.push_back(shaderMetadata);
            }
        }
    }

    return RecluseResult_Ok;
}


ShaderLang getDefaultShaderLanguage()
{
    return kDefaultShaderLanguage;
}
} //