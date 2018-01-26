# getMatrix

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

__all__ = ['main', 'android', 'cmdlines', 'config', 'dataresolve', 'evaluate', 'utils']

if __name__ == "__main__":
    datas = pd.read_csv("res.csv")
    print(datas.columns.values)
    datas.columns.values[0] = "Offset"
    print(datas.columns.values)
    datas.reset_index(drop=True,inplace=True)
    datas = datas.set_index("Offset")
    print(datas)
    datas['a'].plot(color='g')
    plt.plot([20 for i in range(datas['a'].count())],'r-')
    plt.show()
