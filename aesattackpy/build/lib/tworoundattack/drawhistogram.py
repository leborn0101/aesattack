import tworoundattack.utils as utils
import matplotlib.pyplot as plt


#画有和没有aes操作时间直方图
def drawhistogram():
    aestime = utils.readtime()
    aesntime = utils.readntime()
    for i in range(128):
        plt.subplot(16, 8, 1 + i)
        if i < 64:
            plt.hist(aestime[0][i], 100)
        else:
            plt.hist(aesntime[0][i - 64], 100)
    plt.show()














