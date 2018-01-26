'''
用于实现与数据相关的功能
'''
import os
import sqlite3
import numpy as np
import pandas as pd


def initdatabase(databasepath, isSweepData=False):
    '''
    初始化数据库
    '''
    if isSweepData and os.path.exists(databasepath):
        os.remove(databasepath)
    return sqlite3.connect(databasepath)


def readcursor(conn, tablename, indexnames=None, condition=None):
    '''
    从数据库读取数据
    注意：SQL语言中select语句会合并同类项
    '''
    sqlstring = "SELECT "
    if indexnames is None:
        sqlstring += "*"
    else:
        indexstring = None
        for indexname in indexnames:
            if indexstring is not None:
                indexstring += ", " + indexname
            else:
                indexstring = indexname
        sqlstring += indexstring
    tablestring = " FROM " + tablename
    sqlstring += tablestring
    if condition is not None:
        sqlstring += " WHERE " + condition
    print("Run SQL command:\n\t%s" % sqlstring)
    return conn.execute(sqlstring)


def readcolumndatafromcursor(cursor, index):
    '''
    从一个cursor中读取一列数据并返回list
    '''
    datas = []
    for i in cursor:
        datas.append(i[index])
    return datas


def readdffromcursor(cursor, dtypes):
    '''
    dtypes表示的是获得的df的列名列表
    '''
    matrix = list(cursor)
    datas = {
        k: [matrix[i][v] for i in range(0, len(matrix))]
        for k, v in dtypes.items()
    }
    # print(datas)
    return pd.DataFrame(datas, index=datas['index']).drop('index', axis=1)


def read_sql_table_names(tablename, conn):
    '''
    '''
    cursor = conn.cursor()
    cursor.execute("PRAGMA table_info(" + tablename + ")")
    return list(map(lambda x: x[1], cursor.fetchall()))


def read_sql_table(tablename, conn):
    '''
    从数据库中读取数据
    '''
    cursor = readcursor(conn, tablename)
    columns = read_sql_table_names(tablename, conn)
    columns = {v: k for k, v in enumerate(columns)}
    # print(columns)
    return readdffromcursor(cursor, columns)
