# -*- coding:utf-8 -*-
'''
处理评估数据
'''

import functools
import os
import random
import re

import math
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import config
import dataresolve as dr
import database as db


def addtablerows(datatable, data):
    '''
    合并单元格
    '''
    if datatable is None:
        return data
    else:
        return pd.concat([datatable, data], join='outer')


def addtablecolumns(datatable, data):
    '''
    合并单元表格
    '''
    if datatable is None:
        return data
    else:
        return pd.concat([datatable, data], axis=1, join="outer")
        # return pd.merge(
        #     datatable,
        #     data,
        #     how="outer",
        #     on="Offset",
        #     left_index=False,
        #     right_index=False,
        #     copy=True)


def gethistogram(conn):
    '''
    获取所有地址的数据分分布图
    注意：SQL语言中select语句会合并同类项
    '''
    maxhitcount = 350
    datas = pd.Series([0] * maxhitcount, index=range(0, maxhitcount))
    for tablename in config.KEYBOARDINPUTSDICT:
        data = db.readcolumndatafromcursor(db.readcursor(conn, tablename), 2)
        for v in data:
            datas[v] = datas[v] + 1
    return datas


def draw_histogram(datas, pngfilepath=None):
    '''
    将datas绘制直方图
    '''
    datas.plot(grid=True, logx=False)
    newdatas = pd.Series([0.0] * datas.count(), index=datas.index)
    datasum = datas.sum()
    print(datasum)
    count = 0
    for index in datas.index:
        count += datas[index]
        newdatas[index] = count * 1.0 / datasum
    print(newdatas)
    newdatas.plot(secondary_y=True, mark_right=True)
    if pngfilepath is not None:
        plt.savefig(pngfilepath)
    else:
        plt.show()
    plt.clf()


def getdatas(dirpath, conn):
    '''
    读取dirpath文件夹下面所有以.log结尾的文件中的数据并存储在数据库conn中
    '''
    avedatatable = None
    mindatatable = None
    maxdatatable = None
    searchlist = {v: k for k, v in enumerate(config.KEYBOARDINPUTSDICT)}
    # print(searchlist)
    filelist = sorted(
        os.listdir(dirpath),
        key=lambda x: searchlist[x[:-4]] if x[:-4] in searchlist.keys() else -1
    )
    fpattern = re.compile(r'(.+)\.log')
    for file in filelist:
        filepath = dirpath + '/' + file
        if os.path.isdir(filepath) or not filepath.endswith(".log"):
            continue
        filename = fpattern.match(file).group(1)
        # print(filename)
        if os.path.isfile(filepath):
            data = dr.dowithlogfile(filepath, filename)
            # 存储原始数据
            data['original'].to_sql(file[:-4], conn)
            avedatatable = addtablecolumns(avedatatable, data['average'])
            mindatatable = addtablecolumns(mindatatable, data['minimum'])
            maxdatatable = addtablecolumns(maxdatatable, data['maximum'])
    avedatatable.fillna(0).to_sql('average', conn)
    mindatatable.fillna(0).to_sql('minimum', conn)
    maxdatatable.fillna(0).to_sql('maximum', conn)


def distance(a, b):
    '''
    获得两点的欧几里得距离
    '''
    return (a - b) if a > b else (b - a)


def minindex(l):
    '''
    返回list中数值最小的值得index
    '''
    length = len(l)
    if length == 0:
        return
    (mini, minv) = (0, l[0])
    for i in range(1, length):
        if l[i] < minv:
            (mini, minv) = (i, l[i])
    return mini


def getKMeans(datas, k, precision=1):
    '''
    获得数据的k聚类
    '''
    length = len(datas)
    if length == 0:
        return None
    # 初始聚类中心的选择
    average = []
    average.append(datas[math.floor(random.random() * length)])
    for i in range(1, k):
        # print(i)
        dis = 0
        order = 0
        for j in range(0, length):
            tmpd = functools.reduce(
                sum, map(lambda x: math.pow(distance(datas[j], x), 2),
                         average))
            if tmpd > dis:
                tmpd = dis
                order = i
        average.append(datas[order])
    pre = precision + 1
    while pre >= precision:
        matrix = []
        for i in range(0, k):
            fc = functools.partial(distance, average[i])
            matrix.append(list(map(fc, datas)))
        # 转置矩阵
        # print(matrix)
        matrix = list(map(list, zip(*matrix)))
        # print(matrix)
        indexs = list(map(minindex, matrix))
        res = [[] for i in range(0, k)]
        for i, index in enumerate(indexs):
            # print(i, index)
            res[index].append(datas[i])
        old_average = average
        average = list(map(np.average, res))
        pre = sum([distance(old_average[i], average[i]) for i in range(0, k)])
    return res


def getthreshold(data, k=2):
    '''
    获得将数据分为k个聚类的阈值，这里暂时用kMeans算法获得k=2的阈值
    '''
    data = getKMeans(data, k)
    data = sorted(data, key=np.average)
    res = []
    for i in range(1, len(data)):
        l1 = len(data[i - 1])
        l2 = len(data[i])
        if l1 > 0 and l2 > 0:
            res.append((np.max(data[i - 1]) + np.min(data[i])) / 2)
        elif l1 == 0:
            res.append(np.min(data[i]))
        else:
            res.append(np.max(data[i - 1]))
    return res


def evaluate_address(data):
    '''
    评估一个内存地址的关键性
    '''
    threshold = getthreshold(data)
    # print(threshold)
    return threshold[0]


def evaluate_address_key_value(df):
    '''

    Args:
        df: 一行数据

    Returns: ['Offset': evaluate_address(df[1:])]

    '''
    return [df[0], evaluate_address(df[1:])]


def evaluate_addresses(df):
    '''
    评估获得的数据中所有的内存地址的关键性
    '''
    thresholds = df.apply(evaluate_address_key_value, axis=1)
    # print(thresholds)
    return thresholds


def draw_address_hits_graph(dfs, title, ax=None):
    '''
    绘制Hits分布图
    '''
    tmpdf = pd.DataFrame({
        'KBchar':
        dfs['KBchar'].map(config.KEYBOARDINPUTSDICT.index),
        'Hits':
        dfs['Hits']
    })
    tmpdf.plot(
        ax=ax,
        kind="scatter",
        x='KBchar',
        y='Hits',
        title=title,
        xticks=range(0, len(config.KEYBOARDINPUTSDICT)),
        grid=True,
        rot=90).set_xticklabels(config.KEYBOARDINPUTSDICT)


def get_address_hits_data(conn, address):
    '''
    获取某一地址的分布数据
    '''
    datas = None
    dataslength = 1
    for c in config.KEYBOARDINPUTSDICT:
        cursor = db.readcursor(conn, c, condition="Offset='" + address + "'")
        data = db.readcolumndatafromcursor(cursor, 2)
        # print(data)
        length = len(data)
        rows = pd.DataFrame(
            {
                'KBchar': [c] * length,
                "Hits": data
            },
            index=range(dataslength, dataslength + length))
        dataslength += length
        datas = addtablerows(datas, rows)
    return datas


def draw_evaluate_addresses_graphes(conn, line_graph_dir):
    '''
    Args:
        df: pd.DataFrame, the all table contains all the data about the calling
                frequence when every character key is pressed
        graph_dir: string, the path of directory that to store the graphes

    Returns: None

    '''
    if not os.path.exists(line_graph_dir) or not os.path.isdir(line_graph_dir):
        os.makedirs(line_graph_dir)
    # print(df.columns)
    avedf = db.read_sql_table('average', conn).reindex(
        columns=config.KEYBOARDINPUTSDICT).T
    mindf = db.read_sql_table('minimum', conn).reindex(
        columns=config.KEYBOARDINPUTSDICT).T
    maxdf = db.read_sql_table('maximum', conn).reindex(
        columns=config.KEYBOARDINPUTSDICT).T
    # print(df)
    xcount = len(config.KEYBOARDINPUTSDICT)
    for column in avedf.columns:
        threshold = evaluate_address(avedf[column].values)
        t = avedf[column].loc[avedf[column] > threshold].count()
        columndatas = get_address_hits_data(conn, column)
        allthreshold = evaluate_address(columndatas.Hits.values)
        t_all = maxdf[column].loc[maxdf[column] > allthreshold].count()
        if (t > 3 or t == 0) or (t_all > 3 or t_all == 0):
            continue
        tmp = pd.DataFrame({
            'Offset':
            avedf.index,
            'Average':
            avedf[column],
            'Minimum':
            mindf[column],
            'Maximum':
            maxdf[column],
            'Threshold': [threshold] * len(avedf.index.values),
            'AllThreshold': [allthreshold] * len(avedf.index.values)
            # 'Threshold-L': [threshold] * len(avedf.index.values),
            # 'Threshold-R': [threshold] * len(avedf.index.values)
        })
        # ycount = math.ceil(avedf[column].max())
        ycount = math.ceil(maxdf[column].max())
        # 绘制两个图表在同一个子图中
        fig, (axe1, axe2) = plt.subplots(1, 2, sharey=True, figsize=(15, 6))
        tmp.plot(
            ax=axe1,
            grid=True,
            title=column + ' Hits Count',
            xticks=range(0, xcount),
            yticks=range(0, ycount + int(ycount / 30), math.ceil(ycount / 30)),
            # secondary_y=['Maximum', 'Threshold-R'],
            # mark_right=False,
            rot=90)
        # plt.show()
        print("Drawing picture of %s" % column)
        draw_address_hits_graph(
            columndatas, column + ' Hits Count Distribution', ax=axe2)
        plt.savefig(os.path.join(line_graph_dir, column + ".png"))
        plt.clf()


def makematrixes(conn):
    '''
    test
    '''
    threshold = {'Offset': [], 'threshold': []}
    # , 'More': [], 'NotMore': []}
    value = {'Offset': []}
    for c in config.KEYBOARDINPUTSDICT:
        value[c] = []
    # count = {'Offset': []}
    # for c in config.KEYBOARDINPUTSDICT:
    #     count[c] = []
    avedf = db.read_sql_table('average', conn).reindex(
        columns=config.KEYBOARDINPUTSDICT).T
    maxdf = db.read_sql_table('maximum', conn).reindex(
        columns=config.KEYBOARDINPUTSDICT).T
    # print(df)
    for column in avedf.columns:
        t1 = evaluate_address(avedf[column].values)
        t = avedf[column].loc[avedf[column] >= t1].count()
        df = get_address_hits_data(conn, column)
        allthreshold = evaluate_address(df.Hits.values)
        t_all = maxdf[column].loc[maxdf[column] > allthreshold].count()
        if (t > 3 or t == 0) or (t_all > 3 or t_all == 0):
            continue
        t2 = np.int64(allthreshold)
        # print(t2)
        threshold['Offset'].append(column)
        value['Offset'].append(column)
        # count['Offset'].append(column)
        threshold['threshold'].append(t2)
        # threshold['More'].append(df['Hits'].loc[df['Hits'] > t2].count())
        # threshold['NotMore'].append(df['Hits'].loc[df['Hits'] <= t2].count())
        for c in config.KEYBOARDINPUTSDICT:
            c1 = df.loc[(df['KBchar'] == c) & (df['Hits'] >=
                                               t2), :].KBchar.count()
            # c2 = df.loc[(df['KBchar'] == c) & (df['Hits'] <
            #                                    t2), :].KBchar.count()
            t = 0
            if c1 > 0:
                t = 2
            # if df.loc[(df['KBchar'] == c), :].Hits.count() == 0:
            #     t = 1
            value[c].append(t)
            # count[c].append([c1, c2])
    return {
        'threshold': pd.DataFrame(threshold),
        'value': pd.DataFrame(value),
        # 'count': pd.DataFrame(count)
    }


def getkeyaddress(matrix, resaddresses, keyvalue):
    '''
    '''
    for address in matrix.index.values:
        vals = matrix.loc[address, config.KEYBOARDINPUTSDICT]
        matrix.loc[address, 'all'] = np.int64(vals[vals == keyvalue].count())
    matrix.sort_values(by='all', inplace=True)
    # print(len(matrix.index))
    for address in matrix.index.values:
        if len(matrix.columns) == 0:
            continue
        vals = matrix.loc[address, config.KEYBOARDINPUTSDICT]
        if np.int64(vals[vals == keyvalue].count()) != 0:
            resaddresses.append(address)
            matrix.drop(vals[vals == keyvalue].index, axis=1, inplace=True)
            matrix.drop(address, inplace=True)
    # print(matrix)
    # print(resaddresses)
    return matrix, resaddresses


def getkeyaddressesandmatrix(valuematrix):
    '''
    通过矩阵获得关键地址的地址与相关分析矩阵
    '''
    resaddresses = []
    matrix = valuematrix.copy()
    matrix['all'] = 0
    # print(matrix)
    matrix, resaddresses = getkeyaddress(matrix, resaddresses, 2)
    # matrix, resaddresses = getkeyaddress(matrix, resaddresses, 1)
    return valuematrix.loc[resaddresses, :].T


def writeconfigurationfile(configfile, thresholds, matrix):
    ''' 利用得到的阈值和相关矩阵生成配置文件，用于最后结果的检测
    Args:
        configfile: 配置文件的地址
        threshold: 各个地址的阈值
        matrix: 相关矩阵

    Returns: 如果成功返回True，否则返回False

    '''
    writestring = ""
    writestring += "address count:\n" + str(matrix.columns.values.size) + "\n"
    writestring += "character count:\n" + str(matrix.index.values.size) + "\n"
    writestring += "thres:\n" + str(config.ORI_DATA_THRESHOLD) + "\n"
    writestring += "addresses:\n"
    for address in matrix.columns.values:
        writestring += address + "\t"
    writestring += "\n"
    writestring += "thresholds:\n"
    for address in matrix.columns.values:
        writestring += str(thresholds[thresholds['Offset'] == address]
                           ['threshold'].values[0]) + "\t"
    writestring += "\n"
    writestring += "characters:\n"
    for character in matrix.index.values:
        writestring += character + "\t"
    writestring += "\n"
    writestring += "matrixvalues:\n"
    for character in matrix.index.values:
        for address in matrix.loc[character, :].values:
            writestring += str(address) + "\t"
        writestring += "\n"
    with open(configfile, "w") as cfile:
        cfile.write(writestring)


if __name__ == "__main__":
    conn = db.initdatabase("test.db", False)
    # getdatas("/home/larry/Documents/armageddon/makeMatrix/data", conn)
    # draw_evaluate_addresses_graphes(conn, "../graph")
    matrixes = makematrixes(conn)
    # print("Threshold:\n")
    # print(matrixes['threshold'])
    # print("\nValue:\n")
    # print(matrixes['value'])
    matrixes['threshold'].to_csv('../getres/threshold.csv')
    matrixes['value'].to_csv('../getres/values.csv')
    matrix = getkeyaddressesandmatrix(matrixes['value'].set_index('Offset'))
    writeconfigurationfile("config.txt", matrixes['threshold'], matrix)
    conn.close()
    # datas = getdatas("/home/larry/Documents/log")
    # print(datas)
    # datas.to_csv("/home/larry/Documents/log/res/res.log")
