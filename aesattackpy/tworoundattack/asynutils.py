#!/usr/bin/python
import numpy as np
import matplotlib.pyplot as plt


# 返回一个文件的行数-1
def getlinenums(s):
    a = -1
    with open(s, "r") as f:
        while f.readline() != "":
            a = a + 1
    return a


# 返回一个迭代器，每次返回指定文件中的一行数据
# 返回数据格式为n1 n2 n3 n4 ... nN表示某一set m的N次数据
def getlinedata(s):
    with open(s, "r") as f:
        while True:
            dt = []
            line = f.readline().strip()
            data = line.split(" ")
            for each in data:
                if each != '' and ":" not in each:
                    dt.append(eval(each.strip()))
            yield dt


# 获取所有异步攻击数据，每行表示一个set i对应的多次计时结果
def readasyntime(path):
    pt = path + "/asyntime"
    samplelines = getlinenums(pt)
    asyntime = []
    g = getlinedata(pt)
    for i in range(samplelines):
        asyntime.append(next(g))
    return asyntime


# 获取cache每个set的基础时间base time
def getbasetime(path):
    pt = path + "/asynbase"
    base = []
    file = open(pt)
    lines = file.readlines()
    for line in lines:
        elems = line.strip().split(" ")
        tmp = []
        for i in range(len(elems)):
            if elems[i] != "":
                tmp.append(eval(elems[i].strip()))
        tmpmean = np.mean(tmp)
        sum = 0
        count = 0
        # 过滤噪声
        for j in tmp:
            if j < 2 * tmpmean:
                count = count + 1
                sum = sum + j
        base.append(sum / count)
    if len(base) != 512:
        print("error: base time are not whole!!")
    return base


#
def getmeasurementscorebase(path, basetime, base):
    asyntime = readasyntime(path)
    transpose = np.transpose(asyntime)
    avg = []
    for i in range(len(transpose)):
        avg.append(np.mean(transpose[i]))
    for i in range(len(transpose)):
        nu = 0
        tm = 0
        for j in transpose[i]:
            if j < 2 * avg[i]:
                nu = nu + 1
                tm = tm + j
        avg[i] = tm / nu - basetime[base + i]
    return avg


def getmeasurementscore(path):
    asyntime = readasyntime(path)
    transpose = np.transpose(asyntime)
    avg = []
    for i in range(len(transpose)):
        avg.append(np.mean(transpose[i]))
    for i in range(len(transpose)):
        nu = 0
        tm = 0
        for j in transpose[i]:
            if j < 2 * avg[i]:
                nu = nu + 1
                tm = tm + j
        avg[i] = tm / nu
    return avg


def getindex(key, plaintext):
    index = []
    for i in range(len(key)):
        index.append(key[i] ^ plaintext[i])
    return index


def drawasynhist(avg):
    key = [0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7]
    plaintext = [0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6]
    index = getindex(key, plaintext)
    color1 = ['green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green']
    color2 = ['green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green']
    color3 = ['green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green']
    color4 = ['green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green', 'green']
    for i in range(4):
        color1[index[4 * i]] = 'red'
    for i in range(4):
        color2[index[4 * i + 1]] = 'red'
    for i in range(4):
        color3[index[4 * i + 2]] = 'red'
    for i in range(4):
        color4[index[4 * i + 3]] = 'red'
    plt.subplot(2, 2, 1)
    tmp1 = []
    for j in range(16):
        tmp1.append(avg[0 * 16 + j])
    plt.bar(range(16), tmp1, color = color1)
    plt.subplot(2, 2, 2)
    tmp2 = []
    for j in range(16):
        tmp2.append(avg[1 * 16 + j])
    plt.bar(range(16), tmp2, color = color2)
    plt.subplot(2, 2, 3)
    tmp3 = []
    for j in range(16):
        tmp3.append(avg[2 * 16 + j])
    plt.bar(range(16), tmp3, color = color3)
    plt.subplot(2, 2, 4)
    tmp4 = []
    for j in range(16):
        tmp4.append(avg[3 * 16 + j])
    plt.bar(range(16), tmp4, color = color4)
    plt.show()


def doasynattack(path, base):
    basetime = getbasetime(path)
    avg = getmeasurementscorebase(path, basetime, eval(base.strip()))
    drawasynhist(avg)


def doasynattacknobase(path):
    avg = getmeasurementscore(path)
    drawasynhist(avg)