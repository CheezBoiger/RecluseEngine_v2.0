//
#include "ShaderMap.hpp"
#include "Recluse/Messaging.hpp"
#include <unordered_map>

namespace Recluse {


std::unordered_map<std::string, ShaderBuilderFunc> g_shaderBuilderFuncs = 
{ 
    { "glslang",    createGlslangShaderBuilder },
    { "dxc",        createDxcShaderBuilder }
};


ShaderBuilder* createShaderBuilder(const std::string& nameID, ShaderIntermediateCode intermediateCode)
{
    auto& iter = g_shaderBuilderFuncs.find(nameID);
    if (iter != g_shaderBuilderFuncs.end())
    {
        ShaderBuilder* builder = iter->second(intermediateCode);
        return builder;
    }
    R_WARN("ShaderBuilder", "Failed to create a proper shader builder! Name given = %s", nameID.c_str());
    return nullptr;
}
} // Recluse