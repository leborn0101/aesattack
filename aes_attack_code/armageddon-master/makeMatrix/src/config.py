# coding=utf-8

import os
import sys

ORI_DATA_THRESHOLD = 20
ANDROID_DIR_PATH = '/data/local/tmp/'
CACHE_ATTACKS_NAME = 'cache_template_attack'
CACHE_ATTACKS_PATH = os.path.join(ANDROID_DIR_PATH, CACHE_ATTACKS_NAME)
INPUT_SIMULATOR_NAME = 'input-simulator'
INPUT_SIMULATOR = os.path.join(ANDROID_DIR_PATH, INPUT_SIMULATOR_NAME)
ANDROID_LOG_FILE = os.path.join(ANDROID_DIR_PATH, "log/test/")
PROJECT_DIR = os.path.abspath(os.path.join(sys.path[0], '..'))
HOST_LOG_FILE = os.path.join(PROJECT_DIR, "data/")
TMP_FILE_PATH = os.path.join(PROJECT_DIR, "tmp/tmp.log")
KEYBOARDINPUTSDICT = [
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'space', 'enter',
    'backspace', 'none'
]

ADB_PATH = "adb"

LIB_PATH = "/system/lib/libinput.so"
LIB_MAP_START = "b6bc6000"
LIB_MAP_END = "b6be0000"
LIB_MAP_OFFSET = "00000000"

if __name__ == "__main__":
    os.chdir("/home/larry/")
    print(PROJECT_DIR)
    print(TMP_FILE_PATH)
