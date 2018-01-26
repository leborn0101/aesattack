#! /usr/bin/python3
'''
main entry
'''
import multiprocessing
import os
import datetime
import math

import android
import cmdlines
import config


def run_multiprocess(func, args, daemon=True):
    '''
    运行一个新的进程
    '''
    process = multiprocessing.Process(target=func, args=args)
    process.daemon = daemon
    process.start()
    return process


def run_cache_attack(libpath,
                     address,
                     offset,
                     cpuid=0,
                     log_file_path=None,
                     resolveoutfunc=None):
    '''
    运行cache_template_attack
    '''
    commands = [
        config.CACHE_ATTACKS_PATH,
        "-c",
        str(cpuid),
        "-r",
        address,
        "-o",
        offset,
        "-f",
        "1",
        libpath,
    ]
    if log_file_path is not None:
        commands.append("-l")
        commands.append(log_file_path)
    return android.adb_shell(commands, resolveoutfunc)


def run_input_simulator(c, repititions=-1, delay=1, background=False):
    '''
    运行input-simulator，模拟键盘输入
    '''
    if background:
        commands = [
            config.INPUT_SIMULATOR, "-r", str(repititions), "-d", str(delay),
            c, "&"
        ]
    else:
        commands = [
            config.INPUT_SIMULATOR, "-r", str(repititions), "-d", str(delay), c
        ]
    return android.adb_shell(commands)


def getlogfile(filelist, device_log_path, host_log_path):
    '''
    拉取所有的log文件
    '''
    for file in filelist:
        device_file_path = device_log_path + "/" + file + ".log"
        host_file_path = host_log_path + "/" + file + ".log"
        android.adb_pull(device_file_path, host_file_path)


def run_input_simulator_and_cache_attackes(libpath,
                                           addressbegin,
                                           addressend,
                                           offset,
                                           cpuid=0,
                                           log_file_path=None):
    '''
    :param libpath: string, the path of the library the be attacked
    :param addressbegin: string of hex-number,
            the begin address of the library mapping in memory
    :param addressend: string of hex-number,
            the end address of the library mapping in memory
    :param offset: string of hex-number
    :param cpuid: number for cpu id
    :param log_file_path: string of the path of log file
    :return: None
    '''
    for c in config.KEYBOARDINPUTSDICT:
        print("running script for char: %s" % c)
        android.adb_killbyname(config.INPUT_SIMULATOR_NAME)
        android.adb_killbyname(config.CACHE_ATTACKS_NAME)
        print("run input-simulator for char %s" % c)
        if c is not "none":
            run_multiprocess(run_input_simulator, [c, -1, 1, True], True)
        print("run cache attack char %s:" % c)
        run_cache_attack(libpath, addressbegin + "-" + addressend, offset,
                         cpuid,
                         os.path.join(log_file_path, c + ".log"),
                         cmdlines.resolve_cache_attacks)
        print("cache attaks for %s finished, killing input simulator" % c)
    android.adb_killbyname(config.INPUT_SIMULATOR_NAME)
    android.adb_killbyname(config.CACHE_ATTACKS_NAME)


# run_cache_attack(
#     "/system/lib/libinput.so",
#     "b6b19000-b6b33000",
#     "000000000",
#     log_file_path="/data/local/tmp/log/b.log")

# getlogfile(config.KEYBOARDINPUTSDICT, config.ANDROID_LOG_FILE,
#                config.HOST_LOG_FILE)

begintime = datetime.datetime.now()
run_input_simulator_and_cache_attackes(
    config.LIB_PATH,
    config.LIB_MAP_START,
    config.LIB_MAP_END,
    config.LIB_MAP_OFFSET,
    log_file_path="/data/local/tmp/log/test/")
# t = input()
endtime = datetime.datetime.now()
seg = math.floor((endtime - begintime).total_seconds())
print("total used time: %d : %d : %d, total %d second" %
      (math.floor(seg / 3600), math.floor(seg / 60) % 60, seg % 60, seg))
