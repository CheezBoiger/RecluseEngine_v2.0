#
#
#
#
import subprocess, os
import Systems.ConfigureParser as cp

import argparse
import sys

build_systems_dir = os.path.dirname(os.path.realpath(__file__)) + "/Systems"

generate_engine_resources = build_systems_dir + "/GenerateEngineResources.py"
generate_3rdparty_libs = build_systems_dir + "/GenerateThirdPartyLibraries.py"


parsed_commands = None

cmake_generators = {
    "auto": "latest",
    "2022": "Visual Studio 17 2022",
    "2026": "Visual Studio 18 2026"
}


def parse_arguments():
    global parsed_commands
    parser = argparse.ArgumentParser(description="Parsable arguments for the Recluse build system.")
    parser.add_argument("-developer", dest="developer", action="store_true", help="Enable Developer mode for the engine.", default=False)
    parser.add_argument("-meowhash", dest="meowhash", action="store_true", help="Use Meow hash instead of the default XXHash.", default=False)
    parser.add_argument("-update", dest="ft", action="store_true", help="Run update set up, which sets up submodules and/or updates them.", default=False)
    parser.add_argument("-config", dest="config", help="Path and name of configuration ini file.", type=str, default=None)
    parser.add_argument("-initlib", dest="initlib", help="Init the third party libraries.", action="store_true", default=False)
    args = parser.parse_args()
    parsed_commands = args
    return
    
def add_additional_cmake_commands():
    cmds = []

    if parsed_commands.meowhash == True:
        cmds.append("-DR_USE_MEOW_HASH=True")
    else:
        cmds.append("-DR_USE_MEOW_HASH=False")
        
    if parsed_commands.developer == True:
        cmds.append("-DR_DEVELOPER=True")
    else:
        cmds.append("-DR_DEVELOPER=False")
        
    #if parsed_commands.config is not None:
    #    print(f"You typed in: {parsed_commands.config}")
        
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
    
    cmake_predefined_params = os.path.abspath("./Build64/CMakePredefinedCacheParams.cmake")
    config_parser = cp.ConfigureParser()
    cache_created = False
    if parsed_commands.config != "":
        config_dir = parsed_commands.config
        config_parser.read_init(config_dir)
        cache_created = config_parser.generate_cache_file(cmake_predefined_params)
    
    run_submodule_update()
    check_install_package("xxhash")
    
    #subprocess.call(["git", "submodule", "update"])
    party_command = ["py", f"{generate_3rdparty_libs}", "-config", f"{config_parser.get_third_party_config()}"]
    if (parsed_commands.initlib):
        party_command.append("-init")
        
    subprocess.call(party_command)
    subprocess.call(["py", f"{generate_engine_resources}"])
    if not os.path.exists("Build64"):
        os.makedirs("Build64")
    os.chdir("Build64")
    
    additional_cmake_commands = add_additional_cmake_commands()
    
    cmake_commands = ["cmake"]
    
    cmake_commands.extend(additional_cmake_commands)
    if cache_created:
        cmake_commands += ['-C', cmake_predefined_params]
    else:
        cmake_commands += config_parser.generate_option_changes()
    
    cmake_commands.append('..')
    print(cmake_commands)
    subprocess.call(cmake_commands)
    
    # Call test params.
    if config_parser.is_building_test() == True:
        if not os.path.exists("../BuildTest"):
            os.makedirs("../BuildTest")
        os.chdir("../BuildTest")
        test_commands = ["cmake"] + additional_cmake_commands + config_parser.generate_option_changes()
        test_commands.append("../Test")
        subprocess.call(test_commands)
        
    print("Done!")
    os.chdir("..")
    return
    
    
if __name__ == '__main__':
    main()