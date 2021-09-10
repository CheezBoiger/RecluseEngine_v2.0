//
#include "ShaderCompiler.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/ShaderBuilder.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Messaging.hpp"

#include "json.hpp"

#include <string>
#include <sstream>
#include <map>
#include <unordered_map>

using json = nlohmann::json;

namespace Recluse {

// Serialize our enum types with the following for json configs.
NLOHMANN_JSON_SERIALIZE_ENUM(Recluse::ShaderType,
    {
        { Recluse::SHADER_TYPE_VERTEX,          "vertex" },
        { Recluse::SHADER_TYPE_PIXEL,           "pixel" },
        { Recluse::SHADER_TYPE_HULL,            "hull" },
        { Recluse::SHADER_TYPE_DOMAIN,          "domain" },
        { Recluse::SHADER_TYPE_COMPUTE,         "compute" },
        { Recluse::SHADER_TYPE_TESS_CONTROL,    "tessc"},
        { Recluse::SHADER_TYPE_TESS_EVAL,       "tesse" },
        { Recluse::SHADER_TYPE_GEOMETRY,        "geometry"},
        { Recluse::SHADER_TYPE_RAY_ANYHIT,      "anyhit" },
        { Recluse::SHADER_TYPE_RAY_CLOSESTHIT,  "closesthit" },
        { Recluse::SHADER_TYPE_RAY_GEN,         "raygen" },
        { Recluse::SHADER_TYPE_RAY_INTERSECT,   "intersect" },
        { Recluse::SHADER_TYPE_RAY_MISS,        "raymiss" },
        { Recluse::SHADER_TYPE_AMPLIFICATION,   "amp" },
        { Recluse::SHADER_TYPE_MESH,            "mesh" }
    }
)


#define SCENE_BUFFER_DECLARE_STR "RECLUSE_DECLARE_SCENE_BUFFER"
#define SCENE_BUFFER_PARAM_STR "RECLUSE_SCENE_PARAMETER"
#define SCENE_BUFFER_END_STR    "RECLUSE_END_SCENE_BUFFER"

#define SCENE_BUFFER_INCLUDE "RecluseSceneBuffer.h"

#define MAP_SHADER_KEYWORD(keyword, glslKeyword, hlslKeyword) { keyword, { { SHADER_LANG_GLSL, glslKeyword }, { SHADER_LANG_HLSL, hlslKeyword } } }

static std::unordered_map<std::string, std::map<ShaderLang, std::string>> kShaderKeywordMap = {
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


struct ShaderMetaData {
    std::string configs;
    std::string relativefilePath;
    ShaderType shaderType;
    std::map<std::string, void*> preprocess;
};


struct CompilerState {
    std::string name;
    ShaderLang  language;
    std::string outputExt;
    std::string version;
    B32         appendExt;
};


static struct {
    std::string                 sourcePath;
    std::vector<CompilerState>  compilers;
    std::vector<ShaderMetaData> shadersToCompile;
} gConfigs;


static std::string generateShaderSceneView(ShaderLang lang)
{
    ErrType result = REC_RESULT_OK;
    // Scene view buffer string to be passed.
    std::string sceneViewBufferStr  = "#ifndef __RECLUSE_SCENE_BUFFER_H__\n#define __RECLUSE_SCENE_BUFFER_H__\n";
    // Start at this file and navigate to the Engine scene view file.
    std::string kShaderCompilerFile = Filesystem::getDirectoryFromPath(__FILE__);
    // Get the file we need.
    std::string pathToSceneViewFile = kShaderCompilerFile + "/../Engine/Include/Recluse/Renderer/SceneView.hpp";
    
    FileBufferData data             = { };

    result = File::readFrom(&data, pathToSceneViewFile);

    if (result != REC_RESULT_OK) {    
        R_ERR("ShaderCompiler", "Failed to read from file: %s", pathToSceneViewFile.c_str());
        return "";
    }

    // Read the stream.
    std::istringstream iss(data.buffer.data());
    std::string line;
    B32 insideSceneViewStruct = false;

    // Start looking for our scene view declaration struct!
    while (std::getline(iss, line)) {
        
        R_DEBUG("ShaderCompiler", "%s", line.c_str());

        if (line.find(SCENE_BUFFER_PARAM_STR) != std::string::npos && 
            insideSceneViewStruct) {

            // Parameter parsing.
            size_t start    = line.find_first_of('(') + 1;
            size_t end      = line.find_first_of(',');
            size_t sz       = end - start;

            if (sz == 0) {
                R_ERR("ShaderCompiler", "Variable syntax declared, but no name found!");
            }

            std::string varName = line.substr(start, sz);
            start               = end + 1;
            end                 = line.find_first_of(')');
            sz                  = end - start;

            if (sz == 0) {
                R_ERR("ShaderCompiler", "Data type not written!");
            }

            std::string varDataType = line.substr(start, sz);
            varDataType.erase(std::remove_if(varDataType.begin(), varDataType.end(), std::isspace),
                              varDataType.end());
            
            sceneViewBufferStr += "    " + GET(varDataType, lang) + " " + varName + ";\n";
            
            
        } else if (line.find(SCENE_BUFFER_DECLARE_STR) != std::string::npos && 
                   // We also need to make sure we aren't also parsing the macro declaration...
                   line.find("#define") == std::string::npos) {

            insideSceneViewStruct   = true;
            std::string structName  = "";
            size_t start            = line.find_first_of('(') + 1;
            size_t end              = line.find_first_of(')');
            size_t sz               = end - start;

            if (start == std::string::npos || end == std::string::npos) {
                R_ERR("ShaderCompiler", "No struct name provided for scene buffer!");
            }

            structName = line.substr(start, sz);

            sceneViewBufferStr   += GET("struct", lang) + " " + structName + " {\n";
            R_VERBOSE("ShaderCompiler", "Found the scene view declarer!");

        } else if (line.find(SCENE_BUFFER_END_STR) != std::string::npos &&
                   // Check we aren't parsing the macro declaration...
                   line.find("#define") == std::string::npos) {

            insideSceneViewStruct = false;
            R_VERBOSE("ShaderCompiler", "Ending scene view declaration struct...");
            sceneViewBufferStr += " }; \n";

        }
    }

    sceneViewBufferStr += "#endif\n";

    return sceneViewBufferStr;
}

ErrType compileShaders(ShaderLang lang)
{
    std::string sceneViewBufferStructStr    = generateShaderSceneView(lang);
    std::string sourcePath                  = gConfigs.sourcePath;

    R_DEBUG("ShaderCompiler", "Result:\n%s", sceneViewBufferStructStr.c_str());
    ShaderBuilder* pBuilder = createGlslangShaderBuilder(INTERMEDIATE_SPIRV);
    pBuilder->setUp();

    for (auto& shaderMetadata = gConfigs.shadersToCompile.begin(); 
         shaderMetadata != gConfigs.shadersToCompile.end(); 
         ++shaderMetadata) {
        std::string sourceFilePath = sourcePath + "/" + shaderMetadata->relativefilePath;
        Shader* pShader = Shader::create();
        FileBufferData buffer = { };

        ErrType result = File::readFrom(&buffer, shaderMetadata->relativefilePath);

        if (result != REC_RESULT_OK) {
            R_ERR("ShaderCompiler", "Failed to read shader %s...", shaderMetadata->relativefilePath.c_str());
            Shader::destroy(pShader);
            continue;
        } else {

                // Read the file buffer, and check for the scene buffer header include.
                std::string shaderSource = "";
                std::istringstream iss(buffer.buffer.data());
                std::string line;
                while (std::getline(iss, line)) {
                    size_t pos = line.find(SCENE_BUFFER_INCLUDE); 
                    if (pos != std::string::npos) {
                        // We found the include, now replace with the struct header.
                        shaderSource += sceneViewBufferStructStr;
                    } else {
                        shaderSource += line;
                    }
                }    
                
                R_VERBOSE("ShaderCompiler", "%s", shaderSource.c_str());
                result = pBuilder->compile(pShader, shaderSource.c_str(), shaderSource.size(), 
                    lang, shaderMetadata->shaderType);

                if (result == REC_RESULT_OK) {
                    //pShader->saveToFile()
                }
        }

        // Destroy the shader when finished.
        Shader::destroy(pShader);
    }

    pBuilder->tearDown();
    freeShaderBuilder(pBuilder);

    return REC_RESULT_OK;
}


ErrType addShaderToCompile(const std::string& filePath, const std::string& config, ShaderType shaderType)
{
    ShaderMetaData metadata     = { };
    metadata.configs            = config;
    metadata.relativefilePath   = filePath;
    metadata.shaderType         = shaderType;

    gConfigs.shadersToCompile.push_back(metadata);

    return REC_RESULT_OK;
}


ErrType setConfigs(const std::string& configPath)
{
    json jfile;
    {
        Recluse::FileBufferData dat;
        Recluse::ErrType result = Recluse::File::readFrom(&dat, configPath);

        if (result != Recluse::REC_RESULT_OK) {
            R_ERR("ShaderCompiler", "Failed to find config file!");
            return result;
        }

        std::istringstream iss(dat.buffer.data());

        try {
            iss >> jfile;
        } catch (std::exception& e) {
            R_ERR("ShaderCompiler", "%s", e.what());
            return REC_RESULT_FAILED;
        }
    }

    if (jfile.find("compiler") != jfile.end()) {
        auto compilers = jfile["compiler"];

        for (int i = 0; i < compilers.size(); ++i) {
            CompilerState compilerState = { };
            auto compiler               = compilers[i];
            std::string lang            = compiler["language"].get<std::string>();
            std::string name            = compiler["name"].get<std::string>();
            compilerState.name          = name;

            R_INFO("ShaderCompiler", "Name: %s, Language: %s", name.c_str(), lang.c_str());

            if (lang.compare("glsl") == 0) {
                compilerState.language = SHADER_LANG_GLSL;
            } else if (lang.compare("hlsl") == 0) {
                compilerState.language = SHADER_LANG_HLSL;
            }

            gConfigs.compilers.push_back(compilerState);
        }
    }

    return REC_RESULT_OK;
}


ErrType setShaderFiles(const std::string& shadersPath)
{
    json jfile;
    ErrType result = REC_RESULT_OK;
    {
        FileBufferData bufferData = { };
        result = File::readFrom(&bufferData, shadersPath);

        if (result != REC_RESULT_OK) {
            R_ERR("ShaderCompiler", "Failed to open shaders path!");
            return result;        
        }

        std::istringstream iss(bufferData.buffer.data());
        try {
            iss >> jfile;
        } catch (const std::exception& e) {
            R_ERR("ShaderCompiler", "%s", e.what());
            return REC_RESULT_FAILED;
        }
    }

    if (jfile.find("global") != jfile.end()) {
        auto global             = jfile["global"];
        std::string sourcePath  = global["source_path"].get<std::string>();
        gConfigs.sourcePath     = sourcePath;
    }

    if (jfile.find("shaders") != jfile.end()) {
        auto shaders = jfile["shaders"];
        if (shaders.is_array()) {
            size_t sz = shaders.size();
            for (U32 i = 0; i < sz; ++i) {
                std::string name                = shaders[i]["name"].get<std::string>();
                ShaderType shaderType           = shaders[i]["type"].get<ShaderType>();
                ShaderMetaData shaderMetadata   = { };
                shaderMetadata.relativefilePath = name;

                gConfigs.shadersToCompile.push_back(shaderMetadata);
            }
        }
    }
    return REC_RESULT_OK;
}
} //