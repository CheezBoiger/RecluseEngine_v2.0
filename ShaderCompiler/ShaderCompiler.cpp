//
#include "ShaderCompiler.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Messaging.hpp"
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>

namespace Recluse {

#define SCENE_BUFFER_DECLARE_STR "RECLUSE_DECLARE_SCENE_BUFFER"
#define SCENE_BUFFER_PARAM_STR "RECLUSE_SCENE_PARAMETER"
#define SCENE_BUFFER_END_STR    "RECLUSE_END_SCENE_BUFFER"

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


static std::string generateShaderSceneView(ShaderLang lang)
{
    ErrType result = REC_RESULT_OK;
    // Scene view buffer string to be passed.
    std::string sceneViewBufferStr  = "";
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

    return sceneViewBufferStr;
}

ErrType compileShaders(ShaderLang lang)
{
    std::string sceneViewBufferStructStr = generateShaderSceneView(lang);

    R_DEBUG("ShaderCompiler", "Result:\n%s", sceneViewBufferStructStr.c_str());

    return REC_RESULT_OK;
}
} //