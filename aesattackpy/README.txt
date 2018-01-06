created by LiBo.
本工具用于处理attack程序获取到的有关set的计时信息，所需的python版本为3.5

使用方式：
在使用该工具对attack获取的数据进行分析之前，需要先安装分析需要的依赖，安装方式为进入aesattackpy文件夹后执行命令：python3 setup.py install。
待安装完成后，通过aesattack+相应的命令即可完成对数据的分析，能够使用的命令为：
aesattack doaesattack1 -p path //path替换为待分析数据所在文件夹的路径，以下命令同义，第一轮攻击，获取秘钥每个字节的前4位
aesattack doaesattack2 -p path //第二轮攻击，获取秘钥每个字节的后4位，假设获取了第一轮攻击的结果
aesattack attack -p path //完整的进行一次cache攻击，包括第一轮攻击和第二轮攻击，其中第二轮攻击中使用第一轮攻击获取到的秘钥字节的前4位
aesattack asynattack -p path -b base // 进行异步攻击分析 path表示文件所在目录，base表示aes T-table其实位置映射到的cache set索引
aesattack asynattacknobase -p path //同上，不过该分析结果不减去base time