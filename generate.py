#
#
#
#
import subprocess, os

import argparse
import sys

build_systems_dir = os.path.dirname(os.path.realpath(__file__)) + "/BuildSystems"

generate_engine_resources = build_systems_dir + "/GenerateEngineResources.py"


parsed_commands = None


def parse_arguments():
    global parsed_commands
    parser = argparse.ArgumentParser(description="Parsable arguments for the Recluse build system.")
    parser.add_argument("-vulkan", dest="vulkan", action="store_true", help="Enable vulkan", default=False)
    parser.add_argument("-dx11", dest="dx11", action="store_true", help="Enable DX11", default=False)
    parser.add_argument("-dx12", dest="dx12", action="store_true", help="Enable DX12", default=False)
    parser.add_argument("-glsl", dest="glsl", action="store_true", help="Enable GLSlang compiler use.", default=False)
    parser.add_argument("-glsl.older", dest="glsl_older", action="store_true", help="Fallback to an older version of GLSlang compiler api.", default=False)
    parser.add_argument("-dxc", dest="dxc", action="store_true", help="Enable DXC compiler use.", default=False)
    parser.add_argument("-test", dest="test", action="store_true", help="Enable tests.", default=False)
    parser.add_argument("-developer", dest="developer", action="store_true", help="Enable Developer mode for the engine.", default=False)
    parser.add_argument("-xxhash", dest="xxhash", action="store_true", help="Use XXhashing instead of the default Meow Hash.", default=False)
    parser.add_argument("-first-time", dest="ft", action="store_true", help="Run first time set up.", default=False)
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
        cmds.append("-DRCL_GLSLANG=False")

    if parsed_commands.xxhash == True:
        cmds.append("-DR_USE_XXHASH=True")
    else:
        cmds.append("-DR_USE_XXHASH=False")
     
    if parsed_commands.dxc == True:
        cmds.append("-DRCL_DXC=True")
    else:
        cmds.append("-DRCL_DXC=False")
        
    if parsed_commands.glsl_older == True:
        cmds.append("-DR_GLSLANG_LEGACY_API=True")
    else:
        cmds.append("-DR_GLSLANG_LEGACY_API=False")
        
    if parsed_commands.developer == True:
        cmds.append("-DR_DEVELOPER=True")
    else:
        cmds.append("-DR_DEVELOPER=False")
        
    if parsed_commands.dx11 == True:
        cmds.append("-DRCL_DX11=True")
    else:
        cmds.append("-DRCL_DX11=False")
        
    return cmds

def check_install_package(package):
    subprocess.check_call([sys.executable, "-m", "pip", "install", package])
    
def run_submodule_update():
    if parsed_commands.ft == True:
        git_command = ["git", "submodule", "update"]
        git_command.append("--recursive")
        git_command.append("--init")
        subprocess.call(git_command)
    return

def main():
    parse_arguments()
    
    run_submodule_update()
    check_install_package("xxhash")
    
    #subprocess.call(["git", "submodule", "update"])
    subprocess.call(["py", f"{generate_engine_resources}"])
    if not os.path.exists("Build64"):
        os.makedirs("Build64")
    os.chdir("Build64")
    
    additional_cmake_commands = add_additional_cmake_commands()
    
    cmake_commands = ["cmake"]
    
    cmake_commands.extend(additional_cmake_commands)
    cmake_commands.append("..")
    print(cmake_commands)
    subprocess.call(cmake_commands)
    
    # Call test params.
    if parsed_commands.test == True:
        if not os.path.exists("../BuildTest"):
            os.makedirs("../BuildTest")
        os.chdir("../BuildTest")
        subprocess.call(["cmake", "../Test" ])
        
    print("Done!")
    os.chdir("..")
    return
    
    
if __name__ == '__main__':
    main()