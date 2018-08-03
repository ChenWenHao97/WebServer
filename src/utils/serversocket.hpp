#pragma once
#include"init.hpp"
#define READ_BUFF_SIZE 640000
#define WRITE_BUFF_SIZE 640000
using namespace std;

class ServSocket{
    public:
        ServSocket();
        ServSocket(int fd): sockfd(fd) {}
        ~ServSocket();
        bool Socket(int port,string ip_addr);
        bool Accept();
        bool Send(const char * sendbuf,int len);
        bool Recv(char* recvbuf,int len,int timeout);
        int getsockfd();
        int getconnfd();
        bool Setnonblock(bool is);
        bool Closeserv(void);
        bool Closeclient(void);
        bool Setoptions(int option,int value);
        bool Setconnfd(int fd);
    private:
        int port;
        int sockfd;
        int connfd;
        struct sockaddr_in servaddr,cliaddr;
      
};
ServSocket::ServSocket():port(0),sockfd(-1),connfd(-1){
}
ServSocket::~ServSocket()
{
    if(!Closeserv())
    {
        Init::my_err("close err",__LINE__);
    }
}
bool ServSocket::Closeclient()
{
    if(connfd!=-1)
        close(connfd);
    else 
        return false;
    return true;
    
}
bool ServSocket::Closeserv(void)
{
     if(sockfd!=-1)
        close(sockfd);
    else 
        return false;
    
    connfd = sockfd = port =-1;

    return true;
}
bool ServSocket::Setnonblock(bool is)
{
	if(!is)//listenfd
	{
		int old_option = fcntl(sockfd,F_GETFL);
        int new_option = old_option | O_NONBLOCK;
        fcntl(sockfd,F_SETFL,new_option);
        return true;
	}
    else //connfd
    {
        int old_option = fcntl(connfd,F_GETFL);
        int new_option = old_option | O_NONBLOCK;
        fcntl(connfd,F_SETFL,new_option);
        return true;
    }
	return false;

}
bool ServSocket::Setoptions(int option,int value)
{
    bool res = false;
    if(sockfd!=-1)
    {
        res = setsockopt(sockfd,SOL_SOCKET,option,(const void *)&value,sizeof(value));
        //只要value为1，之后设置so_reuseaddr,就不会造成time_wait状态
    }
    return true;
}
bool ServSocket::Socket(int port,string ip_addr)
{
    if(port == -1)
    {
        Init::my_err("port is wrong",__LINE__);
        return false;
    }
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd < 0)
    {
        Init::my_err("create socket is wrong",__LINE__);
        return false;
    }
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,ip_addr.c_str(),&servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    Setoptions(SO_REUSEADDR,1);
    Setoptions(SO_KEEPALIVE,1);//心跳包
    Setoptions(SO_SNDBUF, 640000);
	Setoptions(SO_RCVBUF, 640000);
    if((bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0)
    {
        Init::my_err("bind err",__LINE__);
        return  false;
    }
    if((listen(sockfd,30) < 0))
    {
        Init::my_err("listen err",__LINE__);
        return false;
    }
    return true;

}
bool ServSocket::Accept()
{
    socklen_t len = sizeof(cliaddr);
	if((connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len)) <0)
	{
        Init::my_err("accept err",__LINE__);
		return false;
	}
	return true;

}

bool ServSocket::Send(const char* sendbuf,int len)
{
    if(sockfd < 0 || sendbuf=="" || len < 0)
        return false;
    int left = len,total = 0,res = 0;
    while(left > 0)
    {
        res = send(sockfd,sendbuf+total,left,0);
        if(res < 0)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
                Init::my_err("send failed",__LINE__);
				usleep(3000);//3秒
				res = 0;
			}
		}
        total += res;
        left -= total;
    }
    return total == len;
}
bool ServSocket::Recv(char* recvbuf,int len,int timeout)
{
    if(connfd == -1 || recvbuf == ""||len < 0)
        return false;
    int read_index = 0,byte_read = -1;

    while(read_index!=len)
    {
        byte_read = recv(connfd,recvbuf+read_index,len-read_index,0);
        if(byte_read ==-1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Init::my_err("read failed",__LINE__);
                return false;
            }
        }
        read_index += byte_read;
    }
    return read_index == len;
}
int  ServSocket::getsockfd()
{
    return sockfd;
}
int  ServSocket::getconnfd()
{
    return connfd;
}
bool ServSocket::Setconnfd(int fd)
{
    connfd = fd;
}

