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
using namespace std;

string PATH("/home/cwh/WebServer/src/wwwroot");
//默认wwwroot为根
string ok_200_title = "OK";
sting error_400_title = "Bad Request";
string error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
string error_403_title = "Forbidden";
string error_403_form = "You do not have permission to get file from this server.\n";
string error_404_title = "Not Found";
string error_404_form = "thr requested file was not found on this server.\n";
string error_500_title = "Internal Error";
string error_500_form = "There was an unusal problem serving the requested file.\n";

class task{
    private:
        int connfd;
    public:
        task(ServSocket * socket){
            connfd = socket->getconnfd();
        }

        task(ServSocket &oth): connfd(oth.getconnfd()) {}
        ~task(){
        }
        void response(string message,int status);
        void response_file(int size,int status);
        void response_get(string filename);
        void response_post(string filename);
        // void doit(void);//解析请求头
        void operator()();
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

        cout << "read method is:"<<method<<endl;/////////////////
        cout << "read filename is:"<<filename<<endl;/////////////////
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

            response_get(filename);
        }
        else if(method.find("POST")!=string::npos)//先不写
        {
            /*POST /login.asp HTTP/1.1 
            Host: www.example.com 
            Content-Type: application/x-www-form-urlencoded 
            Content-Length: 35 
            username=wantsoft&password=password //post的数据 */

            string all = temp;
            // all << temp;
            int index = all.find("Content-Length");

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
void task::response(string message,int status)
{
    string buf = "HTTP/1.1 ";
    buf += to_string(status)+" \r\nConnection: Close\r\n"  
		+"content-length:"+ to_string(message.size()) + "\r\n\r\n";
    buf += message;
	write(connfd, buf.c_str(), buf.size());
}
void task::response_file(int size,int status)
{
    string buf = "HTTP/1.1 ";
    buf += to_string(status) + " OK\r\nConnection: Close\r\n"
		+ "content-length:" + to_string(size) + "\r\nContent-Type: "
        +  "text/html" +"\r\n\r\n";
	write(connfd, buf.c_str(), buf.size());//先将html发送过去
}
void task::response_get(string filename)
{
    string full_path;
    full_path = PATH;
    //http://www.google.com/search?hl=en&q=httpclient&btnG=Google+Search&aq=f&oq=
    //动态网页
    bool is_dynamic = false;//网页是否是动态，加没加？
    string argvs;
    int index = -1;

    index = filename.find("?");
    if(index != -1)//动态网页
    {
        argvs = filename.substr(index+1);//一直到尾部
        is_dynamic = true;
    }
    // if(filename.compare("/")==0)
        full_path += "/index.html";
    // else  
    //     full_path += filename;
    
    cout << "full_path is:"<<full_path<<endl;//////////////..

    struct stat filestat;
    int ret = stat(full_path.c_str(),&filestat);//判断文件
    if(ret < 0 || S_ISDIR(filestat.st_mode))//文件不存在
    {
        string no_exit_file;
        // no_exit_file = "<html><title>Tinyhttpd Error</title>
        //                 <body>\r\n
	    //                     404\r\n
		//                 <p>GET: Can't find the file
		//             <hr><h3>The Tiny Web Server<h3></body>";
        no_exit_file = 
R"(<html>
    <head>
        <meta http-equiv="content-type" content="text/html;charset=utf-8">
    </head>
    <body>
        <p>
            <img src="404.jpg" width="75%" height="75%">
        </p>
    </body>
</html>)";
        response(full_path,404);
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
        // int changedir = chdir(full_path.c_str());
        // if(changedir == -1)
        // {
        //     my_err("change dir failed",__LINE__);
        // }
        // cout <<"current dir is :"<<getcwd(NULL,0)<<endl;
        int filefd = open(full_path.c_str(),O_RDONLY);
        if(filefd < 0)
        {
            my_err("open full_path failed",__LINE__);
            return;
        }
        else
        {
            response_file(filestat.st_size,200);//文件的大小和状态码
            sendfile(connfd,filefd,0,filestat.st_size);//0拷贝发送文件，减少不必要的系统调用
        }
    }


}
void response_post(string filename);