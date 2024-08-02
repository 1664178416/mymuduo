

#!/bin/bash
set -e 

#如果没有build目录，创建该目录
if [! -d `pwd`/build]; then
    mkdir `pwd`/build
fi

# 删除build之前内容
rm -rf `pwd`/build/*


#进入build目录
#编译一下
cd `pwd`/build && cmake .. && make

# 回到根目录
cd ..

#吧头文件拷贝到/usr/include/mymuduo so库拷贝到 /usr/lib 目录下
if [ ! -d /usr/include/mymuduo ]; then 
    mkdir /usr/include/mymuduo
fi

#拷贝头文件
for header in `ls *.h`
do 
    cp $header /usr/include/mymuduo
done

#拷贝so库
cp `pwd`/lib/libmymuduo.so /usr/lib

#更新动态库
ldconfig