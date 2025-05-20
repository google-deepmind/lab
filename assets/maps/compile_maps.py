import numpy as np 
import os
from zipfile import ZipFile

from src.environments import DMLabBase

TMP_DIR = "/../../tmp"
# before running this, make sure there are no existing
# dmlab_temp_folder_* folders in the tmp folder from earlier
# builds

DEFAULTDECALFREQUENCY=0.1
DEFAULTRANDOMSEED=1

def get_map_names():
    mypath = os.path.join(os.getcwd(), "precompile_maps/to_be_compiled")
    
    map_names = []
    for f in os.listdir(mypath):
        fname, ext = f.split(".")
        if ext == "txt":
            map_names.append(fname)
    return map_names

def get_map_information(map_name):
    file_name = os.path.join(os.getcwd(), 
                        "precompile_maps/to_be_compiled",
                        map_name+".txt")

    map_info = {'mapName'            : map_name,
                'mapVariationsLayer' : "",
                'decalFrequency'     : DEFAULTDECALFREQUENCY,
                'randomSeed'         : DEFAULTRANDOMSEED}
    with open(file_name, 'r') as file:
        for line in file:
            key,value = line[:-1].split('=')
            if key in ["mapEntityLayer", "mapVariationsLayer", "texture"]:
                value = value[1:-1] #remove quotation_marks
                value = value.replace('\\n','\n')
            else:
                value = float(value)
            map_info[key] = value

    if "P" not in map_info['mapEntityLayer']:
        print("Added spawn location, make sure map has one spawn location.")
        map_info['mapEntityLayer'].replace(" ", "P")
    if "A" not in map_info['mapEntityLayer']:
        print("Added goal location, make sure map has one goal location.")
        map_info['mapEntityLayer'].replace(" ", "A")
    return map_info

def compile_map(map_info):
    env = DMLabBase()

    env.load_map_from_text(map_info['mapName'],
                           map_info['mapEntityLayer'],
                           map_info['mapVariationsLayer'],
                           map_info['decalFrequency'],
                           map_info['texture'],
                           map_info['randomSeed'])

    return env

def check_no_temp_dirs():
    count = 0
    for f in os.listdir(TMP_DIR):
        if f.startswith("dmlab_temp_folder_"):
            count += 1
    return count

def get_tmp_dir():
    for f in os.listdir(TMP_DIR):
        if f.startswith("dmlab_temp_folder_"):
            return os.path.join(TMP_DIR, f, 'baselab')

def extract_bsp_file(map_name):
    mypath = os.path.join(os.getcwd(), "precompile_maps/to_be_compiled")
    tmp_dir = get_tmp_dir()

    for f in os.listdir(tmp_dir):
        zf = ZipFile(os.path.join(tmp_dir, f), 'r')
        zf.extractall(tmp_dir)
        zf.close()
    
    oldplace = os.path.join(tmp_dir, "maps", map_name+".bsp")
    newplace = os.path.join(os.getcwd(), "precompile_maps",
                            "bsp_files", map_name+".bsp")
    # make sure file exists
    f = open(newplace, 'a')
    f.close()

    os.rename(oldplace, newplace)

if __name__ == "__main__":
    if check_no_temp_dirs():
        print("ERROR: existing temp. folders detected.\nDelete these and try again.")
    else:
        # create dir where compiled bsp files will be placed
        if not os.path.exists("precompile_maps/bsp_files"):
            os.makedirs("precompile_maps/bsp_files")
        
        for map_name in get_map_names():
            map_info = get_map_information(map_name)
            env = compile_map(map_info)
            extract_bsp_file(map_name)
            env.lab.close()







