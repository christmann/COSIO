Import("env", "projenv")
from shutil import copyfile
import os
import zipfile
import json

versionPath = "v1"

def move_firmware(source, target, env):
    print("Copying firmware binary to new location /binarys")
    copyfile(".pio/build/d1_mini/firmware.bin", 'binarys/' + versionPath + '/firmware.bin')
    print("Done")

def move_filesystem(source, target, env):
    print("Copying spiffs binary to new location /binarys")
    copyfile(".pio/build/d1_mini/spiffs.bin", 'binarys/' + versionPath + '/spiffs.bin')
    
    print("Zipping /data folder")

    zf = zipfile.ZipFile("binarys/" + versionPath + "/data.zip", "w")
    for dirname, subdirs, files in os.walk("data/"):
        for filename in files:
            zf.write(os.path.join(dirname, filename))
    zf.close()

    print("Done")


with open('data/config.json') as f:
  config = json.load(f)

if config["sensor"]["type"] == 3 :
    versionPath = "v2"

env.AddPostAction(".pio/build/d1_mini/firmware.bin", move_firmware)
env.AddPostAction(".pio/build/d1_mini/spiffs.bin", move_filesystem)
