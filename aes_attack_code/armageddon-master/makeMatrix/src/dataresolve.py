#! /usr/bin/python3
'''
resolve data file
'''
import os
import re
import traceback

import numpy as np
import pandas as pd

import config


def pretreatment(filepath, outfilepath):
    '''
    预处理文件
    '''
    # read_file = open(filepath, "r")
    write_file = open(outfilepath, "w")
    fpattern = re.compile(r'(0x[a-f0-9]+) - ([0-9]+)')
    # print(fpattern)
    with open(filepath, "r") as read_file:
        rescontent = []
        write_file.write("Offset,Hits\n")
        for line in read_file:
            line = line.replace("Offset,Hits\n", "")
            # print(line)
            line = fpattern.findall(line)
            if len(line) == 0:
                continue
            else:
                # print(line)
                write_file.write(line[0][0] + ',' + line[0][1] + '\n')
                # 过滤掉部分较小的数据
                # if int(line[0][1]) > config.ORI_DATA_THRESHOLD or int(line[0][1]) == 0:
                #     rescontent.append(line[0][0] + ',' + line[0][1] + '\n')
        write_file.writelines(rescontent)
        read_file.close()
    write_file.close()


def getGP(n):
    '''
    '''
    GPMap = {
        3: 1.153,
        4: 1.463,
        5: 1.672,
        6: 1.822,
        7: 1.938,
        8: 2.032,
        9: 2.110,
        10: 2.176,
        11: 2.234,
        12: 2.285,
        13: 2.331,
        14: 2.371,
        15: 2.409,
        16: 2.443,
        17: 2.475,
        18: 2.501,
        19: 2.532,
        20: 2.557,
        21: 2.580,
        22: 2.603,
        23: 2.624,
        24: 2.644,
        25: 2.663,
        26: 2.681,
        27: 2.698,
        28: 2.714,
        29: 2.730,
        30: 2.745,
        31: 2.759,
        32: 2.773,
        33: 2.786,
        34: 2.799,
        35: 2.811,
        36: 2.823,
        37: 2.835,
        38: 2.846,
        39: 2.857,
        40: 2.866,
        41: 2.877,
        42: 2.887,
        43: 2.896,
        44: 2.905,
        45: 2.914,
        46: 2.923,
        47: 2.931,
        48: 2.940,
        49: 2.948,
        50: 2.956,
        51: 2.943,
        52: 2.971,
        53: 2.978,
        54: 2.986,
        55: 2.992,
        56: 3.000,
        57: 3.006,
        58: 3.013,
        59: 3.019,
        60: 3.025,
        61: 3.032,
        62: 3.037,
        63: 3.044,
        64: 3.049,
        65: 3.055,
        66: 3.061,
        67: 3.066,
        68: 3.071,
        69: 3.076,
        70: 3.082,
        71: 3.087,
        72: 3.092,
        73: 3.098,
        74: 3.102,
        75: 3.107,
        76: 3.111,
        77: 3.117,
        78: 3.121,
        79: 3.125,
        80: 3.130,
        81: 3.134,
        82: 3.139,
        83: 3.143,
        84: 3.147,
        85: 3.151,
        86: 3.155,
        87: 3.160,
        88: 3.163,
        89: 3.167,
        90: 3.171,
        91: 3.174,
        92: 3.179,
        93: 3.182,
        94: 3.186,
        95: 3.189,
        96: 3.193,
        97: 3.196,
        98: 3.201,
        99: 3.204,
        100: 3.207
    }
    if n <= 3:
        return GPMap[3]
    elif n < 100:
        return GPMap[n]
    else:
        return GPMap[100]


def dataPreprocess(dfs, offsets=None):
    '''
    采用Grubbs方法剔除异常的数据
    '''
    if offsets is None:
        offsets = dfs['Offset'].drop_duplicates().reset_index(drop=True)
    for offset in offsets:
        while True:
            datas = dfs.loc[dfs.Offset == offset].Hits
            # print(datas)
            length = len(datas)
            if length < 3:
                break
            ave = datas.mean()
            std = datas.std()
            maxvalue = datas.max()
            vgp = (maxvalue - ave) / std
            # print(vgp)
            gp = getGP(length)
            # print(gp)
            if vgp > gp:
                dfs.drop(
                    dfs.loc[(dfs.Offset == offset) & (dfs.Hits == maxvalue), :]
                    .index.values,
                    inplace=True)
            else:
                break
    return dfs.reset_index(drop=True)


def readlogfile(filepath, filename):
    '''

    Args:
        filepath: string: 完整的文件路径
        filename: string: 文件名，不包含文件名后缀

    Returns: dict: 字典，包含orignal、average、minimum、maxcimum三个DataFrame类型数据

    '''
    try:
        print("Evaluate log file: " + filename)
        dfs = pd.read_csv(
            filepath, sep=',', dtype={"Offset": np.str,
                                      "Hits": np.int64})
        offsets = dfs['Offset'].drop_duplicates().reset_index(drop=True)
        dfs = dataPreprocess(dfs, offsets)
        count = offsets.count()
        # print(offsets)
        # print(count)
        dfdave = pd.DataFrame({
            filename: pd.Series([0] * count),
            'Offset': offsets
        })
        dfdmin = pd.DataFrame({
            filename: pd.Series([0] * count),
            'Offset': offsets
        })
        dfdmax = pd.DataFrame({
            filename: pd.Series([0] * count),
            'Offset': offsets
        })
        # print(dfdave)
        # print(dfdave.columns)
        for offset in offsets:
            ls = dfs[dfs['Offset'] == offset]["Hits"]
            if len(ls) > 0:
                average = round(ls.mean())
                minvalue = ls.min()
                maxvalue = ls.max()
            else:
                average = 0
                minvalue = 0
                maxvalue = 0
            # varvalue = ls.var() 方差
            dfdave.loc[dfdave.Offset == offset, filename] = average
            dfdmin.loc[dfdmin.Offset == offset, filename] = minvalue
            dfdmax.loc[dfdmax.Offset == offset, filename] = maxvalue
        # print(dfdave)
        dfdave.reset_index(drop=True, inplace=True)
        dfdmin.reset_index(drop=True, inplace=True)
        dfdmax.reset_index(drop=True, inplace=True)
        dfdave = dfdave.set_index("Offset")
        dfdmax = dfdmax.set_index("Offset")
        dfdmin = dfdmin.set_index("Offset")
        return {
            "original": dfs,
            "average": dfdave,
            "maximum": dfdmax,
            "minimum": dfdmin
        }
    except Exception as ex:
        print(Exception, ex)
        traceback.print_exc()
        return None


def dowithlogfile(filepath, filename):
    '''
    do with file
    '''
    tmp_file_path = config.TMP_FILE_PATH
    pretreatment(filepath, tmp_file_path)
    res = readlogfile(tmp_file_path, filename)
    os.remove(tmp_file_path)
    return res


if __name__ == "__main__":
    # dowithlogfile("../data/a.log",'a')
    print(dowithlogfile("../data/a.log", "a"))
