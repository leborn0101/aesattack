import logging
import subprocess
import io

logger = logging.getLogger('default')


def execute_command(command, resolveoutfunc=None):
    '''
    :param command: list for string of the command
    :param resolveoutfunc: function that will be called for each output line of 
                        command 
    :return: subprocess.Popen()
    '''
    proc = subprocess.Popen(
        command, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if resolveoutfunc is None:
        debug = True
        if debug is True:
            for l in io.TextIOWrapper(proc.stdout, encoding='utf-8'):
                print(l, end="")
            for l in io.TextIOWrapper(proc.stderr, encoding='utf-8'):
                print(l, end="")
        else:
            proc.communicate()
    else:
        for l in io.TextIOWrapper(proc.stdout, encoding='utf-8'):
            resolveoutfunc(l)
    return proc
