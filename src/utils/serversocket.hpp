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
#define READ_BUFF_SIZE 640000
#define WRITE_BUFF_SIZE 640000
using namespace std;

class ServSocket{
    public:
        ServSocket();
        ~ServSocket();
        bool Socket(int port,string ip_addr);
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
        struct sockaddr_in servaddr,cliaddr;
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
bool ServSocket::Socket(int port,string ip_addr)
{
    ip = ip_addr;
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
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,ip.c_str(),&servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    Setoptions(SO_REUSEADDR,1);
    Setoptions(SO_KEEPALIVE,1);//心跳包
    Setoptions(SO_SNDBUF, 640000);
	Setoptions(SO_RCVBUF, 640000);
    if((bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0)
    {
        my_err("bind err",__LINE__);
        return  false;
    }
    if((listen(sockfd,30) < 0))
    {
        my_err("listen err",__LINE__);
        return false;
    }
    return true;

}
bool ServSocket::Accept()
{
    socklen_t len = sizeof(cliaddr);
	if((connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len)) <0)
	{
        my_err("accept err",__LINE__);
		return false;
	}
	return true;

}

bool ServSocket::Send(string sendbuf,int len)
{
    if(sockfd < 0 || sendbuf="" || len < 0)
        return false;
    int left = len,total = 0,res = 0;
    while(left > 0)
    {
        res = send(sockfd,sendbuf.c_str()+total,left,0);
        if(ret < 0)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
                my_err("send failed",__LINE__);
				usleep(3000);//3秒
				ret = 0;
			}
		}
        total += res;
        left -= total;
    }
    return total == len;
}
bool ServSocket::Recv(string recvbuf,int len,int timeout)
{
    if(sockfd == -1 || recvbuf == ""||len < 0)
        return false;
    int read_index = 0,byte_read = -1;

    while(read_index!=len)
    {
        byte_read = recv(sockfd,recvbuf.c_str()+read_index,len-read_index,0);
        if(byte_read ==-1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                my_err("read failed",__LINE__);
                return false;
            }
        }
        read_index += byte_read;
    }
    return read_index == len;
}
int  ServSocket::getconnfd()
{
    return connfd;
}
int  ServSocket::getsockfd()
{
    return sockfd;
}

