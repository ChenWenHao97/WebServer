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
#include<sstream>
#include <unordered_map>
using namespace std;
extern unordered_map<std::string, std::string> MimeTypes;

string PATH("/home/cwh/WebServer/src/wwwroot");
//默认wwwroot为根

string ok_200_title = "OK";
string error_400_title = "Bad Request";
string error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
string error_403_title = "Forbidden";
string error_403_form = "You do not have permission to get file from this server.\n";
string error_404_title = "Not Found";
string error_404_form = "thr requested file was not found on this server.\n";
string error_500_title = "Internal Error";
string error_500_form = "There was an unusal problem serving the requested file.\n";

int endsWith(string s,string sub)//从尾部开始查找
{
    return s.rfind(sub)==(s.length()-sub.length())?1:0;
    //长字符串减去短字符串就是对应的索引，从右边开始查找
}
class task{
    private:
        int connfd;
    public:
        task(ServSocket * socket){
            connfd = socket->getconnfd();
        }

        task(ServSocket &oth): connfd(oth.getconnfd()) {}
        void response(string message,string status_title,int status,int size,string content_type);
        void response_get(string filename,string content_type);
        void response_post(string filename);
        void operator()();//请求头
        string filename;
        string method;
        string version;

};
void task::operator()()
{
    char temp[1000];
    int size;
    stringstream buf;

    size = read(connfd,temp,sizeof(temp));
    if(size > 0)
    {
        buf << temp;
        buf >> method;
        buf >> filename;


        // cout <<"temp is :"<<temp<<endl;
        cout << "read method is:"<<method<<endl;/////////////////
        cout << "read filename is:"<<filename<<endl;/////////////////
    
        string all = filename;
        string content_type = "text/html";
     
        for (auto &i: MimeTypes) 
        {
            if (endsWith(all, i.first)) 
            {
                content_type = i.second;
                break;
            }
        }
        // if(endsWith(all,".mp3")) 
        // {
        //     content_type = "audio/mp3";
        // }
        // else if(endsWith(all,".mp4"))
        // {
        //     content_type = "video/mpeg4";
        // }
        // else if(endsWith(all,".jpg"))
        // {
        //     content_type = "image/jpeg";
        // }
        // else if(endsWith(all,".png"))
        // {
        //     content_type = "image/png";
        // }
        // else if(endsWith(all,""))
        
        int type_index = all.find("Content-Type");//要找到需要的类型
        type_index +=12;
        if(method.compare("GET")==0) 
        {
        //  GET /19011.html HTTP/1.1
        //  Host: www.importnew.com
        //  Connection: keep-alive
        //  User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.62 Safari/537.36
        //  Upgrade-Insecure-Requests: 1
        // Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
        //  Accept-Encoding: gzip, deflate
        //  Accept-Language: zh-CN,zh;q=0.9,en;q=0.8

            response_get(filename,content_type);
        }
        else if(method.find("POST")!=string::npos)//先不写
        {
            /*POST /login.asp HTTP/1.1 
            Host: www.example.com 
            Content-Type: application/x-www-form-urlencoded 
            Content-Length: 35 
            username=wantsoft&password=password //post的数据 */

            int length_index = all.find("Content-Length");

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
// void task::response_file(int size,int status)
// {
//     string buf = "HTTP/1.1 ";
//     buf += to_string(status) + " "+status_title+"\r\nConnection: Close\r\n"
// 		+ "content-length:" + to_string(size) + "\r\nContent-Type: "
//         + content_type +"\r\n\r\n";
// 	write(connfd, buf.c_str(), buf.size());//先将html发送过去
// }
void task::response_get(string filename,string content_type)
{

    cout <<"content_type:"<<content_type<<endl;////////////s
    string full_path;
    full_path = PATH;
    int status = 200;//初始化状态码200

    //http://www.google.com/search?hl=en&q=httpclient&btnG=Google+Search&aq=f&oq=
    //动态网页
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