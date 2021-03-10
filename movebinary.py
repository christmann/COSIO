Import("env", "projenv")
from shutil import copyfile
import os
import zipfile

def move_firmware(source, target, env):
    print("Copying firmware binary to new location /binarys")
    copyfile(".pio/build/d1_mini/firmware.bin", 'binarys/firmware.bin')
    print("Done")

def move_filesystem(source, target, env):
    print("Copying spiffs binary to new location /binarys")
    copyfile(".pio/build/d1_mini/spiffs.bin", 'binarys/spiffs.bin')
    
    print("Zipping /data folder")

    zf = zipfile.ZipFile("binarys/data.zip", "w")
    for dirname, subdirs, files in os.walk("data/"):
        for filename in files:
            zf.write(os.path.join(dirname, filename))
    zf.close()

    print("Done")


env.AddPostAction(".pio/build/d1_mini/firmware.bin", move_firmware)
env.AddPostAction(".pio/build/d1_mini/spiffs.bin", move_filesystem)
