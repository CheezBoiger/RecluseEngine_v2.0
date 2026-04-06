
import configparser
import os, sys

cmake_generators = {
    "Auto": "latest",
    "2022": "Visual Studio 17 2022",
    "2026": "Visual Studio 18 2026"
}

class ConfigureParser:
    def __init__(self):
        self.cmake_variables = []
        self.lookup = { }
        self.enable_test = False
        self.third_party_dir = ""
        self.third_party_config_file = ""
        return
        
    def get_third_party_dir(self):
        return self.third_party_dir
        
    def get_third_party_config(self):
        return self.third_party_config_file
        
    def is_building_test(self):
        return self.enable_test
        
    def parse_boolean(self, config, section, opt, cmake_opt):
        if config.has_option(section, opt):
            enable = config.getboolean(section, opt)
            self.cmake_variables.append(tuple([cmake_opt, enable]))
        else:
            self.cmake_variables.append(tuple([cmake_opt, 'False', 'BOOL']))
        return
        
    def read_init(self, file_path):
        cmake_arguments = []
        config = configparser.ConfigParser()
        config.read(os.path.abspath(file_path))
        
        section = "Build"
        if config.has_section(section):
            if config.has_option(section, "Generator"):
                generator = config.get("Build", "Generator")
                print(generator)
                if cmake_generators[generator] != cmake_generators['Auto']:
                    cmake_arguments.append(tuple(['-G', f"{cmake_generators[generator]}"]))
            
        self.parse_boolean(config, "Build", "EnableDX12", "RCL_DX12")
        self.parse_boolean(config, "Build", "EnableVulkan", "RCL_VULKAN")
        self.parse_boolean(config, "Build", "EnableDX11", "RCL_DX11")
        
        self.parse_boolean(config, "Pipeline", "EnableDXC", "RCL_DXC")
        self.parse_boolean(config, "Pipeline", "EnableGLSLANG", "RCL_GLSLANG")
        
        self.parse_boolean(config, "GLSLANG", "EnableLegacy", "RCL_GLSLANG_LEGACY")
        self.parse_boolean(config, "Nvidia", "EnableAftermath", "RCL_ENABLE_AFTERMATH")
        
        if config.has_section("Test"):
            if config.has_option("Test", "Enable"):
                self.enable_test = config.getboolean("Test", "Enable")
                
        if config.has_section("ThirdParty"):
            
            config_name = "Unknown"
            
            if config.has_option("ThirdParty", "ConfigName"):
                config_name = config.get("ThirdParty", "ConfigName")
                
            if config.has_option("ThirdParty", "Path"):
                path = config.get("ThirdParty", "Path")

                if (path == None or path == ""):
                    print(f"No path given for third party libraries, please enter one containing {config_name}")
                    path = input("Enter: ")
                
                if not os.path.isabs(path):
                    print("path given is not absolute.")
                    path = os.path.join(os.getcwd(), path)
                self.third_party_dir = path.replace('\\', "/")
                
            self.third_party_config_file = os.path.join(self.third_party_dir, config_name)
                
        return cmake_arguments
        
    def generate_cache_file(self, cache_path):
        if (not os.path.exists(cache_path)):
            with open(cache_path, "w") as f:
                f.write("# Auto-generated file, do not edit!!!\n")
                for var in self.cmake_variables:
                    enable = "ON" if var[1] == True else "OFF"
                    line = f"set({var[0]} {enable} CACHE BOOL \"\")\n"
                    f.write(line)
                    
                if self.third_party_dir != None:
                    line = f"set(RECLUSE_THIRDPARTY_DIR \"{self.get_third_party_dir()}\" CACHE STRING \"\")\n"
                    f.write(line)
            return True
        return False
     
    def generate_option_changes(self):
        cmake_options = []
        for var in self.cmake_variables:
            enable = "ON" if var[1] == True else "OFF"
            cmake_options.append(f"-D {var[0]}={var[1]}")
            
        cmake_options.append(f"-D RECLUSE_THIRDPARTY_DIR:STRING={self.get_third_party_dir()}")
        return cmake_options
                