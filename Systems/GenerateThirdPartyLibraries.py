#
#
#
#
import subprocess, os
import argparse
import sys

import BuildSystem as BS

build_systems_dir = os.path.dirname(os.path.realpath(__file__)) + "/../Systems"
thirdparty_dir = os.path.dirname(os.path.realpath(__file__)) + "../ThirdParty"

recluse_install_dir = os.path.join(build_systems_dir, "../Recluse")

parsed_commands = None

def parse_arguments():
    global parsed_commands
    parser = argparse.ArgumentParser(description="Generate third party dependencies projects, and compile them.")
    parser.add_argument("-config", dest="config", type=str, help="Path to configuration file for libraries.", default="")
    parser.add_argument("-cache", dest='cache', type=str, help="Path to output the cmake variable cache file.", default="")
    parser.add_argument("-init", dest="init", action="store_true", help="Initialize the library manager system. Should run if first time setup", default=False)
    args = parser.parse_args()
    parsed_commands = args
    return
    
def dump_build_variables(variable_list):
    cmake_var_file_path = os.path.join(recluse_install_dir, "RecluseThirdPartyVars.cmake")
    with open(cmake_var_file_path, "w") as f:
        for var in variable_list:
            variable = var[0]
            word = "SET(" + variable + " "
            values = var[1]
            for value in values:
                word += value + " "
            word += ")\n"
            f.write(word)
    return


def main():
    parse_arguments()
    print(f"Checking Recluse third party lib builds in: {parsed_commands.config}")
    thirdparty_build_dir = build_systems_dir + "/../Build64/ThirdParty"
    # Create and go to this build dir.
    if not os.path.exists("Build64"):
        os.makedirs("Build64")
    os.chdir("Build64")
    
    if not os.path.exists("ThirdParty"):
        os.makedirs("ThirdParty")
    os.chdir("ThirdParty")
    
    # initialize the repo 
    if parsed_commands.init:
        os.chdir(parsed_commands.config)
        subprocess.call(["git", "clone", "https://github.com/CheezBoiger/RecluseLibraries.git"])
        os.chdir("RecluseLibraries")
        subprocess.call(['git', 'submodule', 'update', '--recursive', '--init'])
        os.chdir(thirdparty_build_dir)
        
    buildsystem = BS.BuildSystem()
    buildsystem.set_install_dir(recluse_install_dir)
    
    configure_path = parsed_commands.config
    third_party_libs = buildsystem.parse_configure_file(configure_path)
    # Perform the build scripts here.
    base_path = os.path.dirname(configure_path)
    for path in third_party_libs:
            thirdparty_path = os.path.join(base_path, path)
            files = os.listdir(thirdparty_path)
            for file in files:
                directory_cmake_path = os.path.join(thirdparty_path, path)
                directory = os.path.abspath(path)
                file_path = os.path.join(thirdparty_path, file)
                file_ext = os.path.splitext(file_path)[1]
                if file_ext == ".rbuild":
                    os.chdir(directory_cmake_path)
                    subprocess.call(['git', 'submodule', 'update', '--recursive', '--init'])
                    os.chdir(thirdparty_build_dir)
                    
                    if not os.path.exists(directory):
                        os.makedirs(directory)
                    os.chdir(directory)
                    directory_cmake_path = directory_cmake_path.replace('\\', '/');
                    generate_commands, builds = buildsystem.parse_cmake_commands_from_build_file(file_path, thirdparty_build_dir, thirdparty_path)
                    #print(directory_cmake_path, generate_commands)
                    subprocess.call(["cmake"] + generate_commands + [f"{directory_cmake_path}"])
                    
                    for build in builds:
                        #print(os.getcwd(), build['build'], build['install'])
                        build_name = build['config']
                        print(f'Build Config {build_name} for {directory}')
                        subprocess.call(["cmake", "--build", "."] + build['build'])
                        subprocess.call(["cmake", "--install", "."] + build['install'])
                    os.chdir("..")
    
    #os.chdir("../..")
    
    print(buildsystem.get_build_variables())
    dump_build_variables(buildsystem.get_build_variables())
    return;

if __name__ == '__main__':
    main()