
import subprocess, os

import xml.etree.ElementTree as ET

class BuildSystem:
    def __init__(self):
        self.build_variables = []
        self.install_dir = ""
        return
        
    def get_build_variables(self):
        return self.build_variables
        
    def set_install_dir(self, install_dir):
        self.install_dir = install_dir
        
    def create_initial_cache(self, file_path):
        return
        
    def parse_configure_file(self, file_path):
        build_locations = []
        tree = ET.parse(file_path)
        root = tree.getroot()
        
        if root.tag == "configure":
            for child in root:
                if child.tag == "library":
                    attrib = child.attrib
                    dir_path = attrib['name']
                    build_locations.append(dir_path)
        return build_locations
        
    def parse_library(self, subchild):
        return
        
    def process_build_child(self, child, builds):
        build_commands = []
        install_commands = []
        build_config = ''
        for subchild in child:
            if subchild.tag == "type":
                config = subchild.text
                build_commands += ["--config", config]
                install_commands += ["--config", config]
                build_config = config
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
                                param_text = param_text.replace('\\', '/')
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
                            prefix_dir = prefix_dir.replace("${RECLUSE_INSTALL_PREFIX}", self.install_dir)
                        install_commands += ['--prefix', prefix_dir]
            if subchild.tag == "target":
                target_name = subchild.attrib['name']
                build_commands += ['--target', target_name]
            if subchild.tag == "include":
                print("Nice")
        builds.append({ 'build': build_commands, 'install': install_commands, 'config': build_config })
        
    def parse_cmake_commands_from_build_file(self, build_file_path, build_path, cmake_directory_path):
        tree = ET.parse(build_file_path)
        root = tree.getroot()
        
        generate_commands = [] 
        builds = [] 
        
        if root.tag == "project":
            cmake_directory_path = os.path.join(cmake_directory_path, root.attrib['version'])
            for child in root:
                if child.tag == "build":
                    self.process_build_child(child, builds)
                if child.tag == "param":
                    attrib = child.attrib
                    generate_commands += ['-D', attrib['var'] + "=" + attrib['value']]
                if child.tag == "cache":
                    attrib = child.attrib
                    cache_path = attrib['path']
                    cache_path = cache_path.replace("${RECLUSE_THIRDPARTY_DIRECTORY}", cmake_directory_path)
                    generate_commands += ['-C', cache_path]
                if child.tag == "library":
                    self.parse_library(child)
                if child.tag == "cmake":
                    for subchild in child:
                        if subchild.tag == "param":
                            attrib = subchild.attrib
                            var_name = attrib['name']
                            values = []
                            for node in subchild:
                                if node.tag == "value":
                                    value = node.attrib['name']
                                    if '${RECLUSE_THIRDPARTY_DIRECTORY}' in value:
                                        value = value.replace('${RECLUSE_THIRDPARTY_DIRECTORY}', cmake_directory_path)
                                        value = value.replace('\\', '/')
                                    values.append(value)
                            self.build_variables.append(tuple([var_name, values]))
        return generate_commands, builds