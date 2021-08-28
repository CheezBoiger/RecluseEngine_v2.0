// 
#include "ShaderCompiler.hpp"
#include "Recluse/Messaging.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

enum ArgumentType {
    ARG_TYPE_INT
};

struct ArgValue {
    ArgumentType argType;
    union {
        char*               cstr;
        int                 i32[2];
        short               i16[4];
        long long           i64;
        float               f32;
        double              f64;
        unsigned int        u32[2];
        unsigned short      u16[4];
        unsigned long long  u64;
        unsigned char       u8[8];
        char                i8[8];
        bool                b8[8];
    };
};

unordered_map<string, ArgValue> arguments = {
    { "-s", { ARG_TYPE_INT, 0 }}
};

int main(int c, char* argv[])
{
    Recluse::Log::initializeLoggingSystem();

    Recluse::ErrType result = Recluse::REC_RESULT_OK;

    // Collect our arguments.
    for (int i = 0; i < c; ++i) {

        char* arg = argv[i];
        
        string str = arg;
        
    }


    result = Recluse::compileShaders(Recluse::SHADER_LANG_HLSL);

    if (result != Recluse::REC_RESULT_OK) {

        R_ERR("ShaderCompiler", "Failed to compile some or all shaders!");

        return -1;

    }    

    R_VERBOSE("ShaderCompiler", "Finished!");

    Recluse::Log::destroyLoggingSystem();
    return 0;
}