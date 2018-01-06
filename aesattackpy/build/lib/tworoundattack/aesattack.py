'攻击代码'
import tworoundattack.utils as utils
import matplotlib.pyplot as plt
import click

from tworoundattack import asynutils


def _doaesattack2(first4bits):
    if len(first4bits) == 0:
        frresult = [0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70]
    else:
        frresult = first4bits
    result2 = utils.getmeasurementscore2(frresult)
    result5 = utils.getmeasurementscore5(frresult)
    result8 = utils.getmeasurementscore8(frresult)
    result15 = utils.getmeasurementscore15(frresult)


@click.command()
@click.option('-p', '--path', required=True)
def doaesattack1(path):
    utils.myinit(path)
    result = utils.getmeasurementscorebycache()
    for i in range(16):
        plt.subplot(4, 4, 1 + i)
        plt.bar(range(len(result[i])), result[i])
    pt = path + "/result"
    with open(pt, "w") as f:
        for i in range(16):
            for j in range(16):
                s = "%f" % result[i][j]
                f.write(s)
            f.write("\n")
    plt.show()


@click.command()
@click.option('-p', '--path', required=True)
def doaesattack2(path):
    utils.myinit(path)
    _doaesattack2([])


@click.command()
@click.option('-p', '--path', required=True)
def attack(path):
    utils.myinit(path)
    doaesattack2(path, utils.getmeasurementscorebycache())


@click.command()
@click.option('-p', '--path', required=True)
@click.option('-b', '--base', required=True)
def asynattack(path, base):
    asynutils.doasynattack(path, base)


@click.command()
@click.option('-p', '--path', required=True)
def asynattacknobase(path):
    asynutils.doasynattacknobase(path)


@click.group()
def cli():
    pass


cli.add_command(attack)
cli.add_command(doaesattack1)
cli.add_command(doaesattack2)
cli.add_command(asynattack)
cli.add_command(asynattacknobase)


def main():
    cli(obj={})


if __name__ == '__main__':
    main()