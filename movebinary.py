Import("env", "projenv")
from shutil import copyfile

def move_firmware(source, target, env):
    print("Copying firmware binary to new location /binarys")
    copyfile(".pio/build/d1_mini/firmware.bin", 'binarys/firmware.bin')
    print("Done")

def move_filesystem(source, target, env):
    print("Copying spiffs binary to new location /binarys")
    copyfile(".pio/build/d1_mini/spiffs.bin", 'binarys/spiffs.bin')
    print("Done")


env.AddPostAction(".pio/build/d1_mini/firmware.bin", move_firmware)
env.AddPostAction(".pio/build/d1_mini/spiffs.bin", move_filesystem)
