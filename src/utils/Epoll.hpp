#pragma once
#include<sys/epoll.h>
#include"serversocket.hpp"
#include"threadpool.hpp"
#define MAX_EVENT_NUMBER 1000

using namespace std;

class Epoll{
    public:
        Epoll(int number);
        ~Epoll();
        bool Addevent(ServSocket *socket,bool oneshot);
        bool Deletevent(ServSocket *socket);
        int Waitevent(void);
        struct epoll_event events[MAX_EVENT_NUMBER];
    private:
        int epfd;
};
Epoll::Epoll(int number)
{
    epfd = -1;
    epfd = epoll_create(number);
    if(epfd < 0)
    {
        my_err("create epoll err",__LINE__);
        return;
    }
}
Epoll::~Epoll()
{
    if(epfd > 0)
    {
        close(epfd);
        epfd = -1;
    }
}
bool Epoll::Addevent(ServSocket *socket,bool oneshot)
{
    int sockfd = -1;
    if(socket->Setnonblock())
        sockfd = socket->getsockfd();
    if(sockfd < 0)
    {
        my_err("add epollevent failed",__LINE__);
        return false;
    }
    epoll_event event;
    event.data.fd = sockfd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if(oneshot)
    {
        event.events |= EPOLLONESHOT;
    }
   return ( epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event) == 0);

}
bool Epoll::Deletevent(ServSocket *socket)
{
    int sockfd = socket->getsockfd();
    if(sockfd < 0)
    {
        my_err("delet epollevent failed",__LINE__);
        return false;
    }
    bool res =  (epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,0) == 0);
    close(sockfd);

    return res;
    
}
int Epoll::Waitevent(void)
{
    int ret = epoll_wait(epfd,events,MAX_EVENT_NUMBER,-1);
    return ret;
}