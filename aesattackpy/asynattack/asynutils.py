'工具类'
import numpy as np

# 缓存
samplelines = 0
asyntime = []
avgscore = []


# 获取文件的行数
def getlinenums(s):
    a = -1
    with open(s, "r") as f:
        while f.readline() != "":
            a = a + 1
    return a


# 每次获取文件一行的数据并以数组的形式返回
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


# 获取异步攻击time
def readasyntime(path):
    global samplelines
    global asyntime
    pt = path + "/asyntime"
    g = getlinedata(pt)
    for i in range(samplelines):
        asyntime.append(next(g))
    return asyntime


# 通过plaintext和key获取第一轮访问的索引
def getfirstroundtableaccessindex(plaintext, key):
    index = []
    for i in range(16):
        index[i] = (plaintext[i] ^ key[i]) // 16
    return index


# 获取平均值
def measurementscore(sample):
    return np.mean(sample)


# 获取第一轮攻击的度量分
def getmeasurementscore(path):
    asyntime = readasyntime(path)
    transpose = np.transpose(asyntime)
    tmpavg = []
    for i in range(len(transpose)):
        tmpavg.append(np.mean(transpose[i]))
    for i in range(len(transpose)):
        nu = 0
        tm = 0
        for j in transpose[i]:
            if j < tmpavg[i]:
                nu = nu + 1
                tm = tm + j
        tmpavg[i] = tm / nu
    return tmpavg
