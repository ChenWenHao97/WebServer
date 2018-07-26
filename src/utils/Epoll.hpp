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
        epoll_event * getevents(void);
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
    if(socket->Setnonblock(oneshot))
    {
        if(oneshot)
            sockfd = socket->getconnfd();
        else 
        {
            sockfd = socket->getsockfd();
            cout <<"get socketfd,add event"<<endl;////////////
        }
    }
    
    if(sockfd < 0)
    {
        my_err("add epollevent failed",__LINE__);
        return false;
    }

    struct epoll_event event;
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
    int connfd = socket->getconnfd();
    if(connfd < 0)
    {
        my_err("delet epollevent failed",__LINE__);
        return false;
    }
    bool res =  (epoll_ctl(epfd,EPOLL_CTL_DEL,connfd,0) == 0);
    close(connfd);

    return res;

}
int Epoll::Waitevent(void)
{
    cout <<"after get in waitevent"<<endl;////////////
    int ret = epoll_wait(epfd,events,MAX_EVENT_NUMBER,-1);
    cout <<"after epoll wait "<<endl;////////////
    return ret;
}
epoll_event * Epoll::getevents(void)
{
    return events;
}