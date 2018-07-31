#pragma once
#include<iostream>
#include<string>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<sys/wait.h>
#include<istream>
#include"serversocket.hpp"
#include"my_err.hpp"
#include"../protocol/Httphead.hpp"
#include"../protocol/RequestParser.hpp"
#include"http_parser.h"
#include"http_parser.c"
#include<sstream>
#include <unordered_map>
using namespace std;
extern unordered_map<string,string> ContentTypes;
using byte = unsigned char;//流
string PATH("/home/cwh/WebServer/src/wwwroot");
//默认wwwroot为根

class task{
    private:
        int connfd;
    public:
        task(ServSocket * socket)
        {
            connfd = socket->getconnfd();
        }
        task(ServSocket &oth): connfd(oth.getconnfd()) {}
        void response(string message,string status_title,int status,int size,string content_type);
        void response_get(string filename,string content_type,HttpRequest req);
        void response_post(string filename);
        void operator()();//请求头
        string filename;
        string method;
        string version;
        string contenttype;
};

void task::operator()()
{
    byte temp[1000];
    int size;
    size = read(connfd,temp,sizeof(temp));
    HttpRequest req = RequestParser(temp)();//解析
    method = req.GetMethod();
    version = req.GetVersion();
    filename = req.Geturl();
    if(size > 0)
    {
        
        if(method.compare("GET")==0) 
        {
            response_get(filename,content_type);
        }
        else if(method.find("POST")!=string::npos)//找不到的情况下返回npos,先不写
        {
            response_post(filename,content_type);

        }
        else //未知的方法
        {

        }

    }
    else  if(size < 0)
    {
        cout <<"什么都没读到!!!"<<endl;
    }
    if(connfd > 0)
        close(connfd);

}
void task::response(string message,string status_title,int status,int size,string content_type)
{
    
    string buf = "HTTP/1.1 ";
    buf += to_string(status) + " "+status_title+"\r\nConnection: Close\r\n"
		+ "content-length:" + to_string(size) + "\r\nContent-Type: "
        + content_type +"\r\n\r\n";
    // buf += message;可以直接将内容发过去，不需要再sendfile
    cout <<"response size:"<< size<<endl;
	write(connfd, buf.c_str(), buf.size());//先将html发送过去
}

void task::response_get(string filename,string content_type,HttpRequest req)
{

    cout <<"content_type:"<<content_type<<endl;////////////s
    string full_path;
    full_path = PATH;
    int status = 200;//初始化状态码200

    
    bool is_dynamic = false;//网页是否是动态，加没加？
    string argvs;
    int dynamic_index = -1;

    dynamic_index = filename.find("?");
    if(dynamic_index != -1)//动态网页
    {
        argvs = filename.substr(dynamic_index+1);//一直到尾部
        is_dynamic = true;
    }

        full_path += filename;
    
    cout << "full_path is:"<<full_path<<endl;//////////////..

    struct stat filestat;
    int ret = stat(full_path.c_str(),&filestat);//判断文件
    if(ret < 0 || S_ISDIR(filestat.st_mode))//文件不存在
    {
        full_path = PATH + "/404.html";//只需要将状态改变即可
        stat(full_path.c_str(), &filestat);
        status = 404;
  
    }
    if(is_dynamic)
    {
        if(fork()==0)//创建进程去执行任务
        {
            dup2(connfd,STDOUT_FILENO);//将输出定向到客户端
            execl(full_path.c_str(),argvs.c_str());//执行子程序
        }
        wait(NULL);//等待子进程退出
    }
    else  
    {
        cout <<"get in sendfile"<<endl;/////////////
        int filefd = open(full_path.c_str(),O_RDONLY);
        cout <<"file:size->"<<filestat.st_size<<endl;//////////////
        if(filefd < 0)
        {
            my_err("open full_path failed",__LINE__);
            return;
        }
        else
        {
            response(full_path,ok_200_title,status,filestat.st_size,content_type);//文件的大小和状态码

            ssize_t sended = 0;
            while (sended != filestat.st_size) 
            {
                sended += sendfile(connfd,filefd,&sended,filestat.st_size - sended);
            }
            //0拷贝发送文件，减少不必要的系统调用
            cout << "[SENDED] " << sended << "bytes\n";
        }
        close(filefd);
    }


}
void response_post(string filename);