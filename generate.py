#
#
#
#
import subprocess, os

import argparse

build_systems_dir = os.path.dirname(os.path.realpath(__file__)) + "/BuildSystems"

generate_engine_resources = build_systems_dir + "/GenerateEngineResources.py"


parsed_commands = None


def parse_arguments():
    global parsed_commands
    parser = argparse.ArgumentParser(description="Parsable arguments for the Recluse build system.")
    parser.add_argument("-vulkan", dest="vulkan", action="store_true", help="Enable vulkan", default=False)
    parser.add_argument("-dx12", dest="dx12", action="store_true", help="Enable DX12", default=False)
    parser.add_argument("-glsl", dest="glsl", action="store_true", help="Enable GLSL compilation.", default=False)
    parser.add_argument("-dxil", dest="dxil", action="store_true", help="Enable DXIL compilation.", default=False)
    args = parser.parse_args()
    parsed_commands = args
    return
    
    
def add_additional_cmake_commands():
    cmds = []
    if parsed_commands.vulkan == True:
        cmds.append("-DRCL_VULKAN=True")
    else:
        cmds.append("-DRCL_VULKAN=False")
        
    if parsed_commands.dx12 == True:
        cmds.append("-DRCL_DX12=True")
    else:
        cmds.append("-DRCL_DX12=False")
        
    if parsed_commands.glsl == True:
        cmds.append("-DRCL_GLSLANG=True")
    else:
        cmds.append("-DRCL_GLSLNAG=False")
        
    if parsed_commands.dxil == True:
        cmds.append("-DRCL_DXC=True")
    else:
        cmds.append("-DRCL_DXC=False")
        
    return cmds

def main():
    parse_arguments()

    #subprocess.call(["git", "submodule", "update"])
    subprocess.call(["py", f"{generate_engine_resources}"])
    if not os.path.exists("Build64"):
        os.makedirs("Build64")
    os.chdir("Build64")
    
    additional_cmake_commands = add_additional_cmake_commands()
    
    cmake_commands = ["cmake"]
    
    cmake_commands.extend(additional_cmake_commands)
    cmake_commands.append("..")
    
    subprocess.call(cmake_commands)
    print("Done!")
    os.chdir("..")
    return
    
    
if __name__ == '__main__':
    main()