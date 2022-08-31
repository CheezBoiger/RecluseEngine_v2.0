#
import os, sys, shutil

common_rcl_dir                  = os.path.dirname(os.path.realpath(__file__)) + "/../Engine/Resources"
common_header_generation_dir    = os.path.dirname(os.path.realpath(__file__)) + "/../Recluse/include/Recluse/Generated"

common_preable = \
"""
//
// CONTENTS HERE ARE AUTO-GENERATED. DO NOT EDIT!
#pragma once
#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"    
"""

def check_file_exists(file_path):
    return os.path.isfile(file_path)
    
def generate_directory():
    if (os.path.isdir(common_header_generation_dir)):
        shutil.rmtree(common_header_generation_dir)
        
    os.makedirs(common_header_generation_dir)


def main():
    generate_directory()
    
    for (dirpath, dirnames, filenames) in os.walk(common_rcl_dir): 
        # Generate the directory structure like the Resources.
        for dirname in dirnames:
            new_dir_name = dirpath.split(common_rcl_dir)[1][1:]
            if not os.path.isdir(os.path.join(common_header_generation_dir, new_dir_name, dirname)):
                os.makedirs(os.path.join(common_header_generation_dir, new_dir_name, dirname))
    
        for filename in filenames:
            file_ext = os.path.splitext(filename)
            if ".rcl" in file_ext[1]:
                output_file_name = file_ext[0]
                output_file_name += ".hpp"
                new_dir_path = dirpath.split(common_rcl_dir)[1][1:]
                new_output_path = os.path.join(new_dir_path, output_file_name)
                print(f"Generating {os.path.join(dirpath, filename)} ... {output_file_name}")
                with open(os.path.join(dirpath, filename), "r") as f:
                    lines = f.readlines()
                    with open(os.path.join(common_header_generation_dir, new_output_path), "w") as generated_file:
                        generated_file.write(common_preable)
                        generated_file.writelines(lines)
                
    return

if __name__ == '__main__':
    main()