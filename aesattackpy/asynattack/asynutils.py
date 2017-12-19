#!/usr/bin/python
import numpy as np
import matplotlib.pyplot as plt


def getlinenums(s):
    a = -1
    with open(s, "r") as f:
        while f.readline() != "":
            a = a + 1
    return a


def getlinedata(s):
    with open(s, "r") as f:
        while True:
            dt = []
            line = f.readline().strip()
            data = line.split(" ")
            for each in data:
                if each != '' and "." not in each:
                    dt.append(eval(each.strip()))
            yield dt


def readasyntime(path):
    pt = path + "/asyntime"
    samplelines = getlinenums(pt)
    asyntime = []
    g = getlinedata(pt)
    for i in range(samplelines):
        asyntime.append(next(g))
    return asyntime


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
            if j < avg[i]:
                nu = nu + 1
                tm = tm + j
        avg[i] = tm / nu
    return avg


def drawasynhist(avg):
    key = [0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70]
    plaintext = [0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60]

    plt.subplot(2, 2, 1)
    tmp1 = []
    for j in range(16):
        tmp1.append(avg[0 * 16 + j] - 1500)
    plt.bar(range(16), tmp1)
    plt.subplot(2, 2, 2)
    tmp2 = []
    for j in range(16):
        tmp2.append(avg[1 * 16 + j] - 1500)
    plt.bar(range(16), tmp2)
    plt.subplot(2, 2, 3)
    tmp3 = []
    for j in range(16):
        tmp3.append(avg[2 * 16 + j] - 1500)
    plt.bar(range(16), tmp3)
    plt.subplot(2, 2, 4)
    tmp4 = []
    for j in range(16):
        tmp4.append(avg[3 * 16 + j] - 1500)
    plt.bar(range(16), tmp4)
    plt.show()
