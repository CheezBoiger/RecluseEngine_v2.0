#
#
#
#
import subprocess, os
import argparse
import sys

import xml.etree.ElementTree as ET

build_systems_dir = os.path.dirname(os.path.realpath(__file__)) + "/../Systems"
thirdparty_dir = os.path.dirname(os.path.realpath(__file__)) + "../ThirdParty"

recluse_install_dir = os.path.join(build_systems_dir, "../Recluse")

parsed_commands = None

def parse_arguments():
    global parsed_commands
    parser = argparse.ArgumentParser(description="Generate third party dependencies projects, and compile them.")
    parser.add_argument("-libdir", dest="libdir", help="Library absolute directory.", default="")
    parser.add_argument("-init", dest="init", action="store_true", help="Initialize the library manager system. Should run if first time setup", default=False)
    args = parser.parse_args()
    parsed_commands = args
    return
    
def parse_cmake_commands_from_build_file(build_file_path, build_path, cmake_directory_path):
    tree = ET.parse(build_file_path)
    root = tree.getroot()
    
    generate_commands = [] 
    builds = [] 
    
    if root.tag == "project":
        for child in root:
            if child.tag == "build":
                build_commands = []
                install_commands = []
                for subchild in child:
                    if subchild.tag == "type":
                        config = subchild.text
                        build_commands += ["--config", config]
                        install_commands += ["--config", config]
                    if subchild.tag == "prerun":
                        for prerunChild in subchild:
                            print(prerunChild.tag)
                            if prerunChild.tag == "task":
                                attrib = prerunChild.attrib
                                build_path = os.path.abspath(build_file_path)
                                script_exec = attrib['exec']
                                script_params = []
                                for data in prerunChild:
                                    if data.tag == 'param':
                                        param_text = data.text
                                        param_text = param_text.replace('${RECLUSE_THIRDPARTY_DIRECTORY}', cmake_directory_path)
                                        param_text = param_text.replace('/', '\\')
                                        script_params.append(param_text)
                                command = []
                                if (script_exec != "call"):
                                    command.append(script_exec)
                                command += script_params
                                subprocess.call(command)
                    if subchild.tag == "install":
                        for installchild in subchild:
                            if installchild.tag == "prefix":
                                prefix_dir = installchild.attrib['path']
                                if "${RECLUSE_INSTALL_PREFIX}" in prefix_dir:
                                    prefix_dir = prefix_dir.replace("${RECLUSE_INSTALL_PREFIX}", recluse_install_dir)
                                install_commands = ['--prefix', prefix_dir]
                    if subchild.tag == "include":
                        print("Nice")
                builds.append({ 'build': build_commands, 'install': install_commands })
            if child.tag == "param":
                attrib = child.attrib
                generate_commands += ['-D', attrib['var'] + "=" + attrib['value']]
    return generate_commands, builds

def main():
    parse_arguments()
    print(f"Checking Recluse third party lib builds in: {parsed_commands.libdir}")
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
        os.chdir(parsed_commands.libdir)
        subprocess.call(["git", "clone", "https://github.com/CheezBoiger/RecluseLibraries.git"])
        os.chdir("RecluseLibraries")
        subprocess.call(['git', 'submodule', 'update', '--recursive', '--init'])
        os.chdir(thirdparty_build_dir)
    
    libdir = os.path.join(parsed_commands.libdir, "RecluseLibraries")
    # Perform the build scripts here.
    for path, directories, files in os.walk(libdir):
        for directory in directories:
            print(f"{directory}")
            thirdparty_path = os.path.join(path, directory)
            files = os.listdir(thirdparty_path)
            for file in files:
                directory_cmake_path = os.path.join(thirdparty_path, directory)
                file_path = os.path.join(thirdparty_path, file)
                file_ext = os.path.splitext(file_path)[1]
                if file_ext == ".rbuild":
                    os.chdir(directory_cmake_path)
                    subprocess.call(['git', 'submodule', 'update', '--recursive', '--init'])
                    os.chdir(thirdparty_build_dir)
                    
                    if not os.path.exists(directory):
                        os.makedirs(directory)
                    os.chdir(directory)
                    
                    generate_commands, builds = parse_cmake_commands_from_build_file(file_path, thirdparty_build_dir, directory_cmake_path)
                    subprocess.call(["cmake"] + generate_commands + [f"{directory_cmake_path}"])
                    print(file_path, generate_commands)
                    for build in builds:
                        subprocess.call(["cmake", "--build", "."] + build['build'])
                        subprocess.call(["cmake", "--install", "."] + build['install'])
                        os.chdir("..")
                
        break
    
    #os.chdir("../..")
    return;

if __name__ == '__main__':
    main()