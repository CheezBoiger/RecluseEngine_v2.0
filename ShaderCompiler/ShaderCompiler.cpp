//
#include "ShaderCompiler.hpp"
#include "Recluse/Graphics/Shader.hpp"
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
};


static std::vector<ShaderMetaData> shadersToCompile;


struct CompilerState {
    std::string name;
    ShaderLang  language;
    std::string outputExt;
    std::string version;
    B32         appendExt;
};

struct {
    std::vector<CompilerState> compilers;
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

ErrType compileShaders(const std::string& sourcePath, ShaderLang lang)
{
    std::string sceneViewBufferStructStr = generateShaderSceneView(lang);

    R_DEBUG("ShaderCompiler", "Result:\n%s", sceneViewBufferStructStr.c_str());

    for (auto& shaderMetadata = shadersToCompile.begin(); 
         shaderMetadata != shadersToCompile.end(); 
         ++shaderMetadata) {
        std::string sourceFilePath = sourcePath + "/" + shaderMetadata->relativefilePath;
        Shader* pShader = Shader::create(INTERMEDIATE_SPIRV, shaderMetadata->shaderType);
        FileBufferData buffer = { };

        ErrType result = File::readFrom(&buffer, shaderMetadata->relativefilePath);

        if (result != REC_RESULT_OK) {
            R_ERR("ShaderCompiler", "Failed to read shader %s...", shaderMetadata->relativefilePath.c_str());
            Shader::destroy(pShader);
            continue;
        } else {

                // Read the file buffer, and check for the scene buffer header include.
                std::istringstream iss(buffer.buffer.data());
                std::string line;
                while (std::getline(iss, line)) {
                    if (line.find(SCENE_BUFFER_INCLUDE) != std::string::npos) {
                        // We found the include, now replace with the struct header.
                        continue;
                    }
                }    

        }
    }

    return REC_RESULT_OK;
}


ErrType addShaderToCompile(const std::string& filePath, const std::string& config, ShaderType shaderType)
{
    ShaderMetaData metadata = { };
    metadata.configs = config;
    metadata.relativefilePath = filePath;
    metadata.shaderType = shaderType;

    shadersToCompile.push_back(metadata);

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
            auto compiler = compilers[i];
            std::string lang = compiler["language"].get<std::string>();
            std::string name = compiler["name"].get<std::string>();
            R_INFO("ShaderCompiler", "Name: %s, Language: %s", name.c_str(), lang.c_str());
            compilerState.name = name;

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
} //