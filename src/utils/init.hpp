
#pragma once
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <istream>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <future>
#include <regex>
#include <cstring>
#include <string>
#include <cstdlib>
//read方法需要的头文件
#include <unistd.h>
//socket方法需要的头文件
#include <sys/socket.h>
#include <sys/types.h>
//htonl 方法需要的头文件
#include <netinet/in.h>
//inet_ntop方法需要的头文件
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

std::unordered_map<std::string, std::string> ContentTypes;

class Init
{
  public:
    static void my_err(const char *str, int line);
    static void initContenttypeMap();
    static bool MatchCGI(char *filename);
    static int endsWith(std::string s, std::string sub); //从尾部开始查找
    //设置成静态变量，不用初始化，可以全局使用，只需要Init::方法名 就可以了
};
void Init::my_err(const char *str, int line) //错误处理函数
{
    fprintf(stderr, "line:%d  ", line);
    perror(str);
    std::cout << std::endl;
}
void Init::initContenttypeMap()
{
    FILE *fp = fopen("../protocol/content_type.txt", "r"); //建立以什么结尾，就是什么类型的map

    do
    {
        char key[20], value[50];
        fscanf(fp, " %s %s", key, value);
        ContentTypes[key] = value;
    } while (!feof(fp));

    fclose(fp);
}
bool Init::MatchCGI(char *filename) //匹配用户定义的CGI正则表达式
{

    FILE *fp = fopen("/etc/my_cgi.txt", "r"); //匹配动态的规则
    if (fp == NULL)
    {
        my_err("open cgi.text failed", __LINE__);
    }
    char *rule = new char[512]; //记得delete
    fgets(rule, 512, fp);
    fclose(fp);

    rule[strlen(rule) - 1] = '\0';
    std::cout << "filename is:" << filename << std::endl;
    std::regex reg(rule); //正则匹配
    if (regex_match(filename, reg))
    {
        return true;
    }
    delete[] rule;
    return false;
}
int Init::endsWith(std::string s, std::string sub) //从尾部开始查找
{
    if (s.size() < sub.size()) //当第一个字符串小于第二个字符串长度，一定不是以第二个字符串结尾
        return 0;
    return s.rfind(sub) == (s.length() - sub.length()) ? 1 : 0;
    //长字符串减去短字符串就是对应的索引，从右边开始查找
}
