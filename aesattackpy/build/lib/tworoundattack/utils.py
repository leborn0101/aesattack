#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"包含从文本中读取数据到内存的方法"

from scipy import stats
import numpy as np
from gf256 import GF256


Sbox = (
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16,
)

# 缓存
samplenums = 0
cache = {}
aestime = []
aesntime = []
plaintext = []
allksvalue0 = []
allksvalue1 = []
vessel = []
spth = ''


# 获取文件的行数
def getlinenums(s):
    a = 0
    with open(s, "r") as f:
        while f.readline() != "":
            a = a + 1
    return a


# 获取样本个数
def getsamplenums(s):
    return getlinenums(s) // 64


# 每次获取文件一行的数据并以数组的形式返回
def getlinedata(s):
    with open(s, "r") as f:
        while True:
            dt = []
            line = f.readline().strip()
            data = line.split(" ")
            for each in data:
                if each != '':
                    dt.append(eval(each.strip()))
            yield dt


# 获取aestime
def readtime():
    global spth
    global samplenums
    global aestime
    pt = spth + "/time"
    g = getlinedata(pt)
    for i in range(samplenums):
        aestime.append([])
        for j in range(64):
            aestime[i].append([])
            aestime[i][j] = next(g)
    return aestime


# 获取vessel时间
def readvessel():
    global vessel
    global samplenums
    global aestime
    index = 0
    for i in range(samplenums):
        vessel.append([])
        for j in range(64):
            vessel[i].append(aestime[i][j][index])
    return vessel


# 获取noaestime
def readntime():
    global samplenums
    global aesntime
    global spth
    pt = spth + "/ntime"
    g = getlinedata(pt)
    for i in range(samplenums):
        aesntime.append([])
        for j in range(64):
            aesntime[i].append([])
            aesntime[i][j] = next(g)
    return aesntime


# 读取ksvalue
def readtimeksvalue0():
    global samplenums
    global spth
    pt = spth + "/ks0v"
    aestimeksvalue = []
    g = getlinedata(pt)
    for i in range(samplenums):
        aestimeksvalue.append([])
        aestimeksvalue[i] = next(g)
    return aestimeksvalue


# 读取ksvalue
def readtimeksvalue1():
    global samplenums
    pt = spth + "/ks1v"
    aesnotimeksvalue = []
    g = getlinedata(pt)
    for i in range(samplenums):
        aesnotimeksvalue.append([])
        aesnotimeksvalue[i] = next(g)
    return aesnotimeksvalue


# 离散画ksvalue
def generalksvalue(ksv):
    result = []
    for i in range(len(ksv)):
        result.append([])
        for j in range(len(ksv[i])):
            if (ksv[i][j] > 0.05):
                result[i].append(1)
            else:
                result[i].append(0)
    return result


# 初始化并获取aestime aesntime
def myinit(path):
    global aestime
    global aesntime
    global samplenums
    global plaintext
    global spth
    spth = path
    pt = spth + "/time"
    samplenums = getsamplenums(pt)
    aestime = readtime()
    aesntime = readntime()
    assert len(aesntime) == len(aestime)
    plaintext = readplaintext()[0]
    assert samplenums == readplaintext()[1]
    readvessel()
    getallksvalue()


# 获取ksvalue并存入文件
def getallksvalue():
    global aestime
    global aesntime
    global allksvalue0
    global allksvalue1
    global spth
    ks0v = spth + "/ks0v"
    ks1v = spth + "/ks1v"
    ptw0 = open(ks0v, "w")
    ptw1 = open(ks1v, "w")
    for i in range(len(aestime)):
        allksvalue0.append([])
        allksvalue1.append([])
        assert len(aestime[i]) == len(aesntime[i])
        for j in range(len(aestime[i])):
            ks = stats.ks_2samp(aestime[i][j], aesntime[i][j])
            allksvalue0[i].append(ks[0])
            allksvalue1[i].append(ks[1])
            s0 = "%f " % ks[0]
            s1 = "%f " % ks[1]
            ptw0.write(s0)
            ptw1.write(s1)
        ptw0.write("\n")
        ptw1.write("\n")
        ptw0.flush()
        ptw1.flush()
    ptw0.close()
    ptw1.close()


# 获取plaintext
def readplaintext():
    global plaintext
    global spth
    pt = spth + "/plaintext"
    pn = getlinenums(pt)
    g = getlinedata(pt)
    for i in range(pn):
        plaintext.append([])
        plaintext[i] = next(g)
    return [plaintext, pn]


# 通过plaintext和key获取第一轮访问的索引
def getfirstroundtableaccessindex(plaintext, key):
    index = []
    for i in range(16):
        index[i] = (plaintext[i] ^ key[i]) // 16
    return index


# 通过plaintext和key获取第二轮访问的索引
def getsecondroundtableaccessindex(plaintext, key):
    global Sbox
    x2 = Sbox[plaintext[0] ^ key[0]] ^ Sbox[plaintext[5] ^ key[5]] ^ int(GF256(2) * GF256(Sbox[plaintext[10] ^ key[10]])) ^ int(GF256(3) * GF256(Sbox[plaintext[15] ^ key[15]])) ^ Sbox[key[15]] ^ key[2]
    x5 = Sbox[plaintext[4] ^ key[4]] ^ int(GF256(2) * GF256(Sbox[plaintext[9] ^ key[9]])) ^ int(GF256(3) * GF256(Sbox[plaintext[14] ^ key[14]])) ^ Sbox[plaintext[3] ^ key[3]] ^ Sbox[key[14]] ^ key[1] ^ key[5]
    x8 = int(GF256(2) * GF256(Sbox[plaintext[8] ^ key[8]])) ^ int(GF256(3) * GF256(Sbox[plaintext[13] ^ key[13]])) ^ Sbox[plaintext[2] ^ key[2]] ^ Sbox[plaintext[7] ^ key[7]] ^ Sbox[key[13]] ^ key[0] ^ key[4] ^ key[8] ^ 1
    x15 = int(GF256(3) * GF256(Sbox[plaintext[12] ^ key[12]])) ^ Sbox[plaintext[1] ^ key[1]] ^ Sbox[plaintext[6] ^ key[6]] ^ int(GF256(2) * GF256(Sbox[plaintext[11] ^ key[11]])) ^ Sbox[key[12]] ^ key[15] ^ key[3] ^ key[7] ^ key[11]
    return [x2, x5, x8, x15]


# 获取ksvalue
def kstest(i, index, k):
    global cache
    global aestime
    global aesntime
    key = "%d_%d_%d" % (i, index, k)
    if key in cache:
        return cache[key]
    base = (i % 4) * 16
    index = base + index
    time = aestime[k][index]
    ntime = aesntime[k][index]
    ksvalue = stats.ks_2samp(time, ntime)[0]
    cache[key] = ksvalue
    return ksvalue


# 判断两个数组是否同分布
def isnotsamedistitution(aestime, aesntime):
    histogram0 = []
    histogram1 = []
    for i in range(100):
        histogram0.append(0)
        histogram1.append(0)
    for i in aestime:
        if i // 50 > 99:
            histogram0[99] = histogram0[99] + 1
        else:
            histogram0[i // 50] = histogram0[i // 50] + 1
    for i in aesntime:
        if i // 50 > 99:
            histogram1[99] = histogram1[99] + 1
        else:
            histogram1[i // 50] = histogram1[i // 50] + 1
    m0 = 0
    for i in range(len(histogram0)):
        if histogram0[m0] < histogram0[i]:
            m0 = i
    m1 = 0
    for i in range(len(histogram1)):
        if histogram1[m1] < histogram1[i]:
            m1 = i
    if m0 - m1 >= 8:
        return 1
    return 0


# 获取平均值
def measurementscore(sample):
    return np.mean(sample)


# 获取第一轮攻击的度量分
def getmeasurementscore():
    global plaintext
    # result为二维数组，第一维表示密钥的字节号，第二维表示罗列的密钥值，存储的数值为可信度
    result = []
    # availsample为三围数组，第一，二维与result一致，第三维表示样本索引，其值表示样本的ks值
    availsample = []
    # 最外层循环遍历16个密钥字节
    for i in range(16):
        result.append([])
        availsample.append([])
        # 第二层循环遍历每一个密钥字节可能的取值
        for j in range(16):
            availsample[i].append([])
            # 最内层循环遍历所有样本，找出所有有效样本
            for k in range(samplenums):
                # index为有效样本在对应table中的块偏移数
                index = ((j * 16) ^ plaintext[k][i]) // 16
                availsample[i][j].append(kstest(i, index, k))
            result[i].append(measurementscore(availsample[i][j]))
    return result


#  同上
def getmeasurementscorebycache():
    global plaintext
    global allksvalue0
    result = []
    availsample = []
    for i in range(16):
        result.append([])
        availsample.append([])
        for j in range(16):
            availsample[i].append([])
            for k in range(samplenums):
                index = ((j * 16) ^ plaintext[k][i]) // 16
                availsample[i][j].append(allksvalue0[k][(i % 4) * 16 + index])
            result[i].append(measurementscore(availsample[i][j]))
    return result


def getmeasurementscore2(frresult):
    global vessel
    global samplenums
    global plaintext
    global allksvalue0
    global allksvalue1
    kbase = []
    for i in range(len(frresult)):
        kbase.append(frresult[i])
    # result为四维数组，每一维表示密钥的一个字节的后4位，存储的值为可疑度
    result = []
    for i0 in range(16):
        result.append([])
        kbase[0] = frresult[0] + i0
        for i5 in range(16):
            result[i0].append([])
            kbase[5] = frresult[5] + i5
            for i10 in range(16):
                result[i0][i5].append([])
                kbase[10] = frresult[10] + i10
                for i15 in range(16):
                    kbase[15] = frresult[15] + i15
                    availsample = []
                    # 最内层循环遍历所有样本，找出所有有效样本
                    for k in range(samplenums):
                        # index为有效样本在对应table中的块偏移数
                        index = getsecondroundtableaccessindex(plaintext[k], kbase)[0]
                        index = index // 16
                        availsample.append(allksvalue0[k][32 + index])
                        # availsample.append(vessel[k][32 + index])
                    result[i0][i5][i10].append(measurementscore(availsample))
    return result


def getmeasurementscore5(frresult):
    global vessel
    global samplenums
    global plaintext
    global allksvalue0
    global allksvalue1
    kbase = []
    for i in range(len(frresult)):
        kbase.append(frresult[i])
    # result为四维数组，每一维表示密钥的一个字节的后4位，存储的值为可疑度
    result = []
    for i4 in range(16):
        result.append([])
        kbase[4] = frresult[4] + i4
        for i9 in range(16):
            result[i4].append([])
            kbase[9] = frresult[9] + i9
            for i14 in range(16):
                result[i4][i9].append([])
                kbase[14] = frresult[14] + i14
                for i3 in range(16):
                    kbase[3] = frresult[3] + i3
                    availsample = []
                    # 最内层循环遍历所有样本，找出所有有效样本
                    for k in range(samplenums):
                        # index为有效样本在对应table中的块偏移数
                        index = getsecondroundtableaccessindex(plaintext[k], kbase)[1]
                        index = index // 16
                        availsample.append(allksvalue0[k][16 + index])
                        # availsample.append(vessel[k][16 + index])
                    result[i4][i9][i14].append(measurementscore(availsample))
    return result


def getmeasurementscore8(frresult):
    global vessel
    global samplenums
    global plaintext
    global allksvalue0
    global allksvalue1
    kbase = []
    for i in range(len(frresult)):
        kbase.append(frresult[i])
    # result为四维数组，每一维表示密钥的一个字节的后4位，存储的值为可疑度
    result = []
    for i8 in range(16):
        result.append([])
        kbase[8] = frresult[8] + i8
        for i13 in range(16):
            result[i8].append([])
            kbase[13] = frresult[13] + i13
            for i2 in range(16):
                result[i8][i13].append([])
                kbase[2] = frresult[2] + i2
                for i7 in range(16):
                    kbase[7] = frresult[7] + i7
                    availsample = []
                    # 最内层循环遍历所有样本，找出所有有效样本
                    for k in range(samplenums):
                        # index为有效样本在对应table中的块偏移数
                        index = getsecondroundtableaccessindex(plaintext[k], kbase)[2]
                        index = index // 16
                        availsample.append(allksvalue0[k][index])
                        # availsample.append(vessel[k][index])
                    result[i8][i13][i2].append(measurementscore(availsample))
    return result


def getmeasurementscore15(frresult):
    global vessel
    global samplenums
    global plaintext
    global allksvalue0
    global allksvalue1
    kbase = []
    for i in range(len(frresult)):
        kbase.append(frresult[i])
    # result为四维数组，每一维表示密钥的一个字节的后4位，存储的值为可疑度
    result = []
    for i12 in range(16):
        result.append([])
        kbase[12] = frresult[12] + i12
        for i1 in range(16):
            result[i12].append([])
            kbase[1] = frresult[1] + i1
            for i6 in range(16):
                result[i12][i1].append([])
                kbase[6] = frresult[6] + i6
                for i11 in range(16):
                    kbase[11] = frresult[11] + i11
                    availsample = []
                    # 最内层循环遍历所有样本，找出所有有效样本
                    for k in range(samplenums):
                        # index为有效样本在对应table中的块偏移数
                        index = getsecondroundtableaccessindex(plaintext[k], kbase)[3]
                        index = index // 16
                        availsample.append(allksvalue0[k][48 + index])
                        # availsample.append(vessel[k][48 + index])
                    result[i12][i1][i6].append(measurementscore(availsample))
    return result


class Node(object):
    def __init__(self, i, j, k, l, s):
        self.i = i
        self.j = j
        self.k = k
        self.l = l
        self.s = s


def getrank(result):
    array = []
    for i0 in range(16):
        for i1 in range(16):
            for i2 in range(16):
                for i3 in range(16):
                    array.append(Node(i0, i1, i2, i3, result[i0][i1][i2][i3]))
    sortarray(array)
    return array


def sortarray(array):
    count = len(array)
    for i in range(1, count):
        key = array[i]
        j = i - 1
        while j >= 0:
            if array[j].s > key.s:
                array[j + 1] = array[j]
                array[j] = key
            j -= 1
    return array


def getresult(r2, r5, r8, r15):
    r_0_5_10_15 = getrank(r2)
    r_4_9_14_3 = getrank(r5)
    r_8_13_2_7 = getrank(r8)
    r_12_1_6_11 = getrank(r15)
    global spth
    pt = spth + "/r_0_5_10_15"
    with open(pt, "w") as f:
        for i in range(65536):
            t = r_0_5_10_15[i]
            if (t.i == 0 and t.j == 5 and t.k == 2 and t.l == 7):
                print("rank r_0_5_10_15 is %d\n" % i)
            s = "%3d%3d%3d%3d %10.8f" % (t.i, t.j, t.k, t.l, t.s)
            f.write(s)
            f.write("\n")
    pt = spth + "/r_4_9_14_3"
    with open(pt, "w") as f:
        for i in range(65536):
            t = r_4_9_14_3[i]
            if (t.i == 4 and t.j == 1 and t.k == 6 and t.l == 3):
                print("rank r_4_9_14_3 is %d\n" % i)
            s = "%3d%3d%3d%3d %10.8f" % (t.i, t.j, t.k, t.l, t.s)
            f.write(s)
            f.write("\n")
    pt = spth + "/r_8_13_2_7"
    with open(pt, "w") as f:
        for i in range(65536):
            t = r_8_13_2_7[i]
            if (t.i == 0 and t.j == 5 and t.k == 2 and t.l == 7):
                print("rank r_8_13_2_7 is %d\n" % i)
            s = "%3d%3d%3d%3d %10.8f" % (t.i, t.j, t.k, t.l, t.s)
            f.write(s)
            f.write("\n")
    pt = spth + "/r_12_1_6_11"
    with open(pt, "w") as f:
        for i in range(65536):
            t = r_12_1_6_11[i]
            if (t.i == 4 and t.j == 1 and t.k == 6 and t.l == 3):
                print("rank r_12_1_6_11 is %d\n" % i)
            s = "%3d%3d%3d%3d %10.8f" % (t.i, t.j, t.k, t.l, t.s)
            f.write(s)
            f.write("\n")











