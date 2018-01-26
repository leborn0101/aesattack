'''
android 
'''
import config
import cmdlines
from utils import execute_command


def adb_shell(command, resolveoutfunc=None):
    command.insert(0, config.ADB_PATH)
    command.insert(1, "shell")
    return execute_command(command, resolveoutfunc)


def adb_pull(device_file_path, host_file_path):
    execute_command(["adb", "pull", device_file_path, host_file_path])


def adb_kill_by_pid(pid):
    adb_shell(['kill', pid])


def adb_resolve_kill(outline):
    cmdlines.resolve_ps_and_kill(outline, adb_kill_by_pid)


def adb_killbyname(name):
    adb_shell(["ps", "|", "grep", name], resolveoutfunc=adb_resolve_kill)
