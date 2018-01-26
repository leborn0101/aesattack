'''
一个用于处理命令行输入输出的函数库
'''

import re
import sys
import time

CACHE_ATTACKS_PATTERN = re.compile(r'(0x[a-f0-9]+) - ([0-9]+)')
PS_PATTERN = re.compile(
    r"(\w+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\w+)\s+(\w+)\s+(\w+)\s+" +
    r"([\w/\.-]+)")


def resolve_cache_attacks(outline):
    '''
    解析cache_template_attacks的输出，在命令行中刷新显示进度条
    '''
    line = CACHE_ATTACKS_PATTERN.findall(outline)
    if len(line) is not 0:
        percent = int(int(line[0][0], 16) * 100 / int('0x1a000', 16))
        outstring = str(percent) + " % || " + "[" + "#" * percent
        if percent < 99:
            if 1 == (int(time.time()) % 2):
                outstring += '-->'
            else:
                outstring += ' - '
            outstring += " " * (97 - percent) + "]\r"
        else:
            outstring += " " * (100 - percent) + "]\r"
        sys.stdout.write(outstring)
        sys.stdout.flush()

def resolve_ps(outline):
    '''
    解析ps命令的输出获得进程的各种信息，这里只获得简单的几个参数
    '''
    line = PS_PATTERN.findall(outline)
    if len(line) is not 0:
        # print("user: %s" % line[0][0])
        # print("pid: %s" % line[0][1])
        # print("path: %s" % line[0][8])
        return {'user': line[0][0], 'pid': line[0][1], 'path': line[0][8]}
    else:
        return None


def resolve_ps_and_kill(outline, killbypidfunc):
    '''
    解析ps命令的输出获得进程的pid，并杀死进程
    '''
    tinfo = resolve_ps(outline)
    if tinfo is not None:
        killbypidfunc(tinfo['pid'])


if __name__ == "__main__":
    print(int('0x0', 16))
    resolve_cache_attacks("0x0 - 2")
    resolve_ps(("root      32256 16117 2572   988   hrtimer_na b6f1c824 ",
                "S /data/local/tmp/input-simulator"))
