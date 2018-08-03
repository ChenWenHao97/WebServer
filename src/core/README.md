## main函数
### makefile的书写

分为测试版本（DEBUG）和正式版本（RELEASE）</br>

①前几行都是给依赖起了名字</br>

CC = g++(使用的编译器)</br>
DEBUG = -g （使用gdb调试的时候加的参数）</br>
RELEASE = -O2 （优化）</br>
LIBRARY = -lpthread（使用线程必须要链接动态库）</br>
FILE = webserver（生成的别名）</br>


②在编译的时候使用{ }包含前面依赖的名字</br>

③使用
直接输入make，然后./webserver

④删除.o文件
make clean




