libflush_k51c78_WDTF为同步攻击的源代码
    依赖：
        本项目需要依赖ndk工具，首先需要下载android ndk工具，并将config-arm.mk以及config-arm64.mk中的ANDROID_NDK_PATH环境变量指向ndk所在的文件夹
    编译方式：
        1.cd到 ./libflush_k51c78_WDTF/ 文件夹下执行：sudo make ARCH=armv8
        2.cd到 ./libflush_k51c78_WDTF/example/ 文件夹下执行：ndk-build NDK_APPLICATION_MK=`pwd`/Application.mk NDK_PROJECT_PATH=`pwd`，将在./libflush_k51c78_WDTF/example/libs/arm64-v8a/文件加下生成attack_interfaces可执行文件，通过push libs/arm64-v8a/* data/local/tmp/命令将attack_interfaces文件push到目标机下的data/local/tmp/文件夹下
        3.通过adb shell进入目标机的shell命令窗口，执行cd /data/local/tmp进入到该文件夹下，执行./attack_interfaces加参数即可进行同步攻击并生成数据，参数如下所示：
            -f <value> (可选mode1， mode2， mode3， 为攻击模式，其中mode2效果较好，默认使用mode1)
            -c <value> (绑定到的cpu,默认cpu 0)
            -s <value> (攻击的样本个数，默认位2000个样本)
            -r <value> (每轮攻击重复次数，默认位100，增到会提高结果的准确性)
            -k <value> (设置aes秘钥，该参数为字符串格式，且必须为"0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,0x77, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,"这种模式，每个秘钥字节以逗号相隔，且不要少了最后一个逗号，默认为刚刚这个值)
            -h 显示命令参数信息
        4.执行完毕后，在电脑端控制台通过adb pull data/local/tmp文件夹下的所有数据导入到指定文件夹下后通过python编译生成的aesattack工具进行分析

libflush_k51c78_ASYN为异步攻击的源代码
    依赖：
        本项目需要依赖ndk工具，首先需要下载android ndk工具，并将config-arm.mk以及config-arm64.mk中的ANDROID_NDK_PATH环境变量指向ndk所在的文件夹
    编译方式：
        1.cd到 ./libflush_k51c78_ASYN/ 文件夹下执行：sudo make ARCH=armv8
        2.cd到 ./libflush_k51c78_ASYN/example/ 文件夹下执行：ndk-build NDK_APPLICATION_MK=`pwd`/Application.mk NDK_PROJECT_PATH=`pwd`，将在./libflush_k51c78_ASYN/example/libs/arm64-v8a/文件加下生成asynattack_test可执行文件，通过push libs/arm64-v8a/* data/local/tmp/命令将asynattack_test文件push到目标机下的data/local/tmp/文件夹下
        3.通过adb shell进入目标机的shell命令窗口，执行cd /data/local/tmp进入到该文件夹下，执行./asynattack_test加参数即可进行同步攻击并生成数据，参数如下所示：
            -c <value> (绑定到的cpu,默认cpu 0)
            -m <value> (0表示为获取基础时间模式，1表示攻击模式)
            -d <value> (延迟时间，表示prime操作和probe操作的延迟时间)
            -s <value> (攻击的总次数)
            -b <value> (攻击模式下设置table起始位置对应的set号)
            -h 显示命令参数信息
        4.执行完毕后，在电脑端控制台通过adb pull data/local/tmp文件夹下的所有数据导入到指定文件夹下后通过python编译生成的aesattack工具进行分析

libflush_k51c78_victim为异步被攻击程序的源代码
    依赖：
        本项目需要依赖ndk工具，首先需要下载android ndk工具，并将config-arm.mk以及config-arm64.mk中的ANDROID_NDK_PATH环境变量指向ndk所在的文件夹
    编译方式：
        1.cd到 ./libflush_k51c78_victim/ 文件夹下执行：sudo make ARCH=armv8
        2.cd到 ./libflush_k51c78_victim/example/ 文件夹下执行：ndk-build NDK_APPLICATION_MK=`pwd`/Application.mk NDK_PROJECT_PATH=`pwd`，将在./libflush_k51c78_victim/example/libs/arm64-v8a/文件加下生成victim可执行文件，通过push libs/arm64-v8a/* data/local/tmp/命令将victim文件push到目标机下的data/local/tmp/文件夹下
        3.通过adb shell进入目标机的shell命令窗口，执行cd /data/local/tmp进入到该文件夹下，执行./victim加参数即可进行同步攻击并生成数据，参数如下所示：
            -c <value> (绑定到的cpu,默认cpu 0)
            -h 显示命令参数信息
        4.执行完毕后，在电脑端控制台通过adb pull data/local/tmp文件夹下的所有数据导入到指定文件夹下后通过python编译生成的aesattack工具进行分析


