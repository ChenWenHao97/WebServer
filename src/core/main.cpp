#include<iostream>
#include"my_err.hpp"
#include"serversocket.cpp"
#include"threadpool.hpp"
#include"Epoll.hpp"

#define PORT 8080
#define IP "127.0.01"
class test{

};
using namespace std;
int main()
{
    ServSocket socket_object;
    Epoll epoll_object;
    threadpool<test> *pool = NULL;
    try{
        pool = new thread<test>;
    }catch(...){
        return 1;
    }

    socket_object.Socket();
    epoll_object.Epoll(10);
    epoll_object.Addevent(sockfd.getsockfd(),false);

    while(true)
    {
        int number = epoll_object.Waitevent();
        if(number < 0 && errno != EINTR)
        {
            my_err("get epoll number failed",__LINE__);
            break;
        }
       
        for(int i = 0 ;i < number ;i++)
        {
           
            int sockfd = (socket_object.getevents())[i].data.fd;
            if(sockfd == sockfd.getsockfd())
            {
                bool connstate =  socket_object.Accept();
                if(connstate == false)
                {
                  my_err("accept connstate failed",__LINE__);
                  continue;
                }
                int connfd = socket_object.getcnnfd();
                
                epoll_object.Addevent(,true);

            }
            
        }
       
    }
}
