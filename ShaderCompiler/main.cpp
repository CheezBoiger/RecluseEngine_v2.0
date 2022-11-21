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
        ("f,file", "File containing all shaders and their options, to compile.", cxxopts::value<string>()),
        ("i,idx", "Compiler index.", cxxopts::value<int>()->default_value("0"));
    options.allow_unrecognised_options();

    auto parsed = options.parse(c, argv);

    string configPath;
    string compilePath;
    int compilerIndex = 0;

    try 
    {
        configPath       = parsed["config"].as<string>();
        compilePath      = parsed["file"].as<string>();
        if (parsed.count("idx"))
            compilerIndex    = parsed["idx"].as<int>();
    } 
    catch (const exception& e) 
    {
        R_ERR("ShaderCompiler", "%s", e.what());
    }

    Recluse::setConfigs(configPath, compilerIndex);

    Recluse::ErrType result = Recluse::RecluseResult_Ok;

    // Collect our arguments.
    Recluse::setShaderFiles(compilePath);
    result = Recluse::compileShaders(Recluse::ShaderLang_Glsl);

    if (result != Recluse::RecluseResult_Ok) 
    {
        R_ERR("ShaderCompiler", "Failed to compile some or all shaders!");

        return -1;
    }    

    R_VERBOSE("ShaderCompiler", "Finished!");

    Recluse::Log::destroyLoggingSystem();
    return 0;
}