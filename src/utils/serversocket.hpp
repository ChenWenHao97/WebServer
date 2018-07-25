#include<iostream>
#include<cstdlib>
#include<string>
#include<cstring>
//read方法需要的头文件
#include<unistd.h>
//socket方法需要的头文件
#include<sys/socket.h>
#include<sys/types.h>
//htonl 方法需要的头文件
#include<netinet/in.h>
//inet_ntop方法需要的头文件
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>
#include"my_err.hpp"

using namespace std;

class ServSocket{
    public:
        ServSocket();
        ~ServSocket();
        bool Socket(int port);
        bool Send(string sendbuf,int len);
        bool Recv(string recvbuf,int len,int timeout);
        int getconnfd();
        int getsockfd();
        bool Accept();

    private:
        string ip;
        int port;
        int sockfd;
        int connfd;
        struct sockaddr_in servaddr;
        bool Close();
        bool Setnonblock(int fd);
        bool Setoptions(int option,int value);
};
ServSocket::ServSocket():port(0),sockfd(-1),connfd(-1)
{
    ip = "";
}
ServSocket::~ServSocket()
{
    if(!Close())
    {
        my_err("close err",__LINE__);
    }
}
bool ServSocket::Close()
{
    if(connfd!=-1)
        close(connfd);
    else 
        return false;
    if(sockfd!=-1)
        close(sockfd);
    else 
        return false;
    sockfd = connfd = port = -1;
    return true;
    
}
bool ServSocket::Setnonblock(int fd)
{
    if(fd!=-1)
    {
        int old_option = fcntl(fd,F_GETFL);
        int new_option = old_option | O_NONBLOCK;
        fcntl(fd,F_SETFL,new_option);
        return true;
    }
    return false;
}
bool ServSocket::Setoptions(int option,int value)
{
    bool res = false;
    if(sockfd!=-1)
    {
        res = setsockopt(sockfd,SOL_SOCKET,option,(const void *)value,sizeof(value));
        //只要value为1，之后设置so_reuseaddr,就不会造成time_wait状态
    }
    return true;
}
bool ServSocket::Socket(int port)
{
    if(port == -1)
    {
        my_err("port is wrong",__LINE__);
        return false;
    }
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd < 0)
    {
        my_err("create socket is wrong",__LINE__);
        return false;
    }
}
bool ServSocket::Accept()
{

}

bool ServSocket::Send(string sendbuf,int len);
bool ServSocket::Recv(string recvbuf,int len,int timeout);
int  ServSocket::getconnfd();
int  ServSocket::getsockfd();
