//
#include "ShaderCompiler.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Messaging.hpp"
#include <string>
#include <sstream>

namespace Recluse {

std::string kSceneViewHeader = "";
#define SCENE_BUFFER_DECLARE_STR "RECLUSE_DECLARE_SCENE_BUFFER"
#define SCENE_BUFFER_PARAM_STR "RECLUSE_SCENE_PARAMETER"
#define SCENE_BUFFER_END_STR    "RECLUSE_END_SCENE_BUFFER"

static void generateShaderSceneView()
{
    ErrType result = REC_RESULT_OK;
    // Start at this file and navigate to the Engine scene view file.
    std::string kShaderCompilerFile = Filesystem::getDirectoryFromPath(__FILE__);
    // Get the file we need.
    std::string pathToSceneViewFile = kShaderCompilerFile + "/../Engine/Include/Recluse/Renderer/SceneView.hpp";
    
    FileBufferData data = { };

    result = File::readFrom(&data, pathToSceneViewFile);

    if (result != REC_RESULT_OK) {    
        R_ERR("ShaderCompiler", "Failed to read from file: %s", pathToSceneViewFile.c_str());
        return;
    }

    // Read the stream.
    std::istringstream iss(data.buffer.data());
    std::string line;
    B32 insideSceneViewStruct = false;

    // Start looking for our scene view declaration struct!
    while (std::getline(iss, line)) {
        
        R_DEBUG("ShaderCompiler", "%s", line.c_str());

        if (line.find(SCENE_BUFFER_DECLARE_STR) != std::string::npos) {
            insideSceneViewStruct = true;
            R_VERBOSE("ShaderCompiler", "Found the scene view declarer!");
        }

        if (line.find(SCENE_BUFFER_END_STR) != std::string::npos) {
            insideSceneViewStruct = false;
            R_VERBOSE("ShaderCompiler", "Ending scene view declaration struct...");
        }
    }
}

ErrType compileShaders()
{
    generateShaderSceneView();

    return REC_RESULT_OK;
}
} //