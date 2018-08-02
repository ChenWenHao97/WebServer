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
#include"../protocol/http_parser.h"
#include<sstream>
#include <regex>
#include <unordered_map>
using namespace std;
extern unordered_map<string,string> ContentTypes;
extern unordered_map<HttpVersion,string> version_map;
extern unordered_map<HttpMethod,string> method_map;
using byte = unsigned char;//流
string PATH("/home/cwh/WebServer/src/wwwroot");
string ok_200_title = "OK";
string error_400_title = "Bad Request";
string error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
string error_403_title = "Forbidden";
string error_403_form = "You do not have permission to get file from this server.\n";
string error_404_title = "Not Found";
string error_404_form = "thr requested file was not found on this server.\n";
string error_500_title = "Internal Error";
string error_500_form = "There was an unusal problem serving the requested file.\n";
//默认wwwroot为根

int endsWith(string s,string sub)//从尾部开始查找
{
    return s.rfind(sub)==(s.length()-sub.length())?1:0;
    //长字符串减去短字符串就是对应的索引，从右边开始查找
}
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
        void response_post(string filename,string content_type,HttpRequest req);
        void operator()();//请求头
        char* fastcgi(FastCgi_t *c,string filename,char* method,HttpRequest req);
        string filename;
        string method;
        string version;
        string contenttype;
};
char * task::fastcgi(FastCgi_t *c,string filename,char * method,HttpRequest req)
{
    c = (FastCgi_t *)malloc(sizeof(FastCgi_t));
    FastCgi_init(c);
    setRequestId(c,1);
    startConnect(c);
    sendStartRequestRecord(c);

    sendParams(c, "SCRIPT_FILENAME", const_cast<char *>(filename.data()));
    sendParams(c, "REQUEST_METHOD",  method);//const_cast<char *>(method.data()));
    sendParams(c, "CONTENT_LENGTH", (char *)to_string(req.GetBodySize()).c_str());
    sendParams(c, "CONTENT_TYPE", (char *)req.GetValue("Content-Type").c_str());
    sendEndRequestRecord(c);
    if(req.GetBodySize()!=-1)
    {
        auto t = makeHeader(5, 1, req.GetBodySize(), 0);
        //制造头为了发body
        send(c->sockfd_, &t, sizeof(t), 0);
        send(c->sockfd_, req.GetBody(), req.GetBodySize(), 0);
        t = makeHeader(5, 1, 0, 0);
        //说明body发完了
        send(c->sockfd_, &t, sizeof(t), 0);
    }
    // dup2(STDOUT_FILENO,connfd);//将输出直接在页面显示
    char * html = readFromPhp(c);
    FastCgi_finit(c);
    return html;
}

void task::operator()()
{
    cout << "START!" << endl;////////////
    byte temp[1000];
    int size;
    size = read(connfd,temp,sizeof(temp));
    cout <<temp<<endl;/////////////////
    HttpRequest req = HttpRequestParser(temp)();//解析
    HttpMethod method = req.GetMethod();
    version = version_map[req.GetVersion()];
    filename = req.Geturl();
    
    if(size > 0)
    {
        string content_type = "text/html";
     
        for (auto &i: ContentTypes) 
        {
            if (endsWith(filename, i.first)) 
            {
                content_type = i.second;
                break;
            }
        }
        
        switch (method)
        {
        case (HttpMethod::GET):
            response_get(filename,content_type,req);
            break;
        case (HttpMethod::POST):
            response_post(filename,content_type,req);
            break;
        default:
            cout <<"方法还没实现"<< endl;
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
    cout <<"response size:"<< size<<endl;
	write(connfd, buf.c_str(), buf.size());//先将html发送过去
}

void task::response_get(string filename,string content_type,HttpRequest req)
{
    req.SetBody(NULL,-1);
    cout<<"GET!!!"<<endl;//////////////
    string full_path;
    full_path = PATH;
    int status = 200;//初始化状态码200
    bool is_dynamic = false;//网页是否是动态，加没加？

    FILE * fp =  fopen("/etc/my_cgi.txt","r");//匹配动态的规则
    if(fp == NULL)
    {
        my_err("open cgi.text failed",__LINE__);
    }
    char * rule = new char[512]; //记得delete
    fgets(rule,512,fp);
    fclose(fp);
    rule[strlen(rule) - 1] = '\0';
    cout <<"filename is:"<<filename<<endl;
    regex reg(rule);//正则匹配
    if (regex_match(filename, reg))
    {
        is_dynamic = true;
        cout <<endl;
    }

    full_path += filename;
    if(filename[filename.size()-1]=='/')
        full_path += "index.html";
    
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
        //cgi
        // if(fork()==0)//创建进程去执行任务
        // {
        //     dup2(connfd,STDOUT_FILENO);//将输出定向到客户端
        //     execl(full_path.c_str(),argvs.c_str());//执行子程序
        // }
        // wait(NULL);//等待子进程退出

        cout <<endl;/////////////
        cout <<"dynamic!!!!!!!!"<<endl;//////////
        //fastcgi
        FastCgi_t * c;
       
        char *html = fastcgi(c,full_path,"GET",req);
        cout <<"html:"<<endl;//////////////
        cout <<html<<endl;///////////
        response(full_path,ok_200_title,status,strlen(html),content_type);
        int sended = 0;
        while (sended != strlen(html)) 
        {
            sended += send(connfd,html,strlen(html)-sended,0);;
        }
        free(html);//虽然之前动态分配了，不断通过拷贝来到这里，
        //但是释放内存的时候是根据地址进行释放，所以同样有效
    }
    else  
    {
        // cout <<"get in sendfile"<<endl;/////////////
        int filefd = open(full_path.c_str(),O_RDONLY);
        // cout <<"file:size->"<<filestat.st_size<<endl;//////////////
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
        }
        close(filefd);
    }

}
void task::response_post(string filename,string content_type,HttpRequest req)
{
    
    cout <<"POST!!!"<<endl;////////////
    // string full_path = "/home/cwh/WebServer/src/FastCGI"+filename; 
    string full_path = PATH + filename;
    FastCgi_t * c;
    byte * body = req.GetBody();

    char *html = fastcgi(c,full_path,"POST",req);
    cout <<"html:"<<endl;//////////////
    cout <<html<<endl;///////////
    response(full_path,ok_200_title,200,strlen(html),content_type);
    int sended = 0;
    while (sended != strlen(html)) 
    {
        sended += send(connfd,html,strlen(html)-sended,0);;
    }
    free(html);//虽然之前动态分配了，不断通过拷贝来到这里，
    // 但是释放内存的时候是根据地址进行释放，所以同样有效  

    // if(fork() == 0) 
	// /*创建子进程执行对应的子程序，多线程中，创建子进程，
	// 只有当前线程会被复制，其他线程就“不见了”，这正符合我们的要求，
	// 而且fork后执行execl，程序被进程被重新加载*/
	// {
	// 	dup2(connfd, STDOUT_FILENO);  
	// 	//将标准输出重定向到sockfd,将子程序输出内容写到客户端去。
	// 	execl(full_path.c_str(), argv); //执行子程序
	// }
	// wait(NULL);

}