'攻击代码'
import tworoundattack.utils as utils
import matplotlib.pyplot as plt
import click


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


def _doaesattack2(path, tmp):
    if len(tmp) == 0:
        frresult = [0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70]
    else:
        frresult = tmp
    result2 = utils.getmeasurementscore2(frresult)
    result5 = utils.getmeasurementscore5(frresult)
    result8 = utils.getmeasurementscore8(frresult)
    result15 = utils.getmeasurementscore15(frresult)


@click.command()
@click.option('-p', '--path', required=True)
def doaesattack2(path):
    utils.myinit(path)
    _doaesattack2(path, [])


@click.command()
@click.option('-p', '--path', required=True)
def attack(path):
    utils.myinit(path)
    doaesattack2(path, utils.getmeasurementscorebycache())


@click.command()
@click.option('-n', '--name', required=True)
def hello(name):
    print("hello" + name)
 

@click.group()
def cli():
    pass

cli.add_command(attack)
cli.add_command(doaesattack1)
cli.add_command(doaesattack2)
cli.add_command(hello)


def main():
    cli(obj={})

if __name__ == '__main__':
    main()