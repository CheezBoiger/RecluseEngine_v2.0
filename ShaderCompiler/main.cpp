// 
#include "ShaderCompiler.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include "cxxopts.hpp"

#include <string>
#include <istream>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;


int main(int c, char* argv[])
{
    Recluse::Log::initializeLoggingSystem();

    cxxopts::Options options("Recluse ShaderCompiler", u8"Shader Compiler for Recluse Shaders.");
    
    options.add_options()
        ("c,config", "Metadata configuration file for compiler.", cxxopts::value<string>())
        ("f,file", "File containing all shaders and their options, to compile.", cxxopts::value<string>());
    options.allow_unrecognised_options();

    auto parsed = options.parse(c, argv);

    string configPath;
    string compilePath;

    try {
        configPath       = parsed["config"].as<string>();
        compilePath      = parsed["file"].as<string>();
    } catch (const exception& e) {
        R_ERR("ShaderCompiler", "%s", e.what());
    }

    Recluse::setConfigs(configPath);

    Recluse::ErrType result = Recluse::REC_RESULT_OK;

    // Collect our arguments.
    Recluse::setShaderFiles(compilePath);
    result = Recluse::compileShaders(Recluse::SHADER_LANG_HLSL);

    if (result != Recluse::REC_RESULT_OK) {

        R_ERR("ShaderCompiler", "Failed to compile some or all shaders!");

        return -1;

    }    

    R_VERBOSE("ShaderCompiler", "Finished!");

    Recluse::Log::destroyLoggingSystem();
    return 0;
}