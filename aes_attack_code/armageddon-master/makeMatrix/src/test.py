'''
一个用于测试的类
'''

import unittest

import evaluate as el
import database as db


class test_mkmatrix(unittest.TestCase):
    def test_gethistogram(self):
        '''
        none
        '''
        conn = db.initdatabase('test.db')
        datas = el.gethistogram(conn)
        print(datas)
        el.draw_histogram(datas)

    def test_getKMeans(self):
        '''
        测试getKMeans() 方法
        '''
        self.assertListEqual(
            el.getKMeans([1, 2, 3, 4, 6, 7, 8, 9], 2, 1),
            [[1, 2, 3, 4], [6, 7, 8, 9]])
        self.assertListEqual(
            el.getKMeans([1, 2, 3, 4, 5, 6, 7, 8, 9], 2, 1),
            [[1, 2, 3, 4], [5, 6, 7, 8, 9]])

    def test_evaluate(self):
        '''
        测试evaluate.py文件中的代码
        '''
        self.test_getKMeans()

    def test_database(self):
        '''
        测试database中的代码
        '''
        conn = db.initdatabase('test.db')
        cursor = db.readcursor(
            conn, 'a', indexnames=['Hits'], condition="Offset='0x1a40'")
        print(db.readcolumndatafromcursor(cursor, 0))


if __name__ == "__main__":
    test_mkmatrix().test_gethistogram()
