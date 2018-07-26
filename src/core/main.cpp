#include<iostream>
#include"../utils/my_err.hpp"
#include"../utils/serversocket.hpp"
#include"../utils/threadpool.hpp"
#include"../utils/Epoll.hpp"
#include"../utils/task.hpp"
#define PORT 8080
#define IP "127.0.01"

class test{

};
using namespace std;
int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        cout <<"usage :"<<argv[0]<<"port"<<endl;
        return 1;
    }

    ServSocket socket_object;
   

    threadpool pool(20);//线程池
    cout <<"after pool"<<endl;////////////
    int port = atoi(argv[2]);
    string ip = argv[1];
    socket_object.Socket(port,ip);
    cout <<"after socket"<<endl;////////////
    Epoll epoll_object(1000);
    cout <<"after epoll"<<endl;////////////
    bool a = epoll_object.Addevent(&socket_object,false);
    cout <<"after epoll add socket#####" << (a?"true":"false") <<endl;////////////
    while(true)
    {
        int number = epoll_object.Waitevent();
        cout <<"epoll_wait number is :"<<number<<endl;//////////
        if(number < 0 && errno != EINTR)
        {
            my_err("get epoll number failed",__LINE__);
            break;
        }
       
        for(int i = 0 ;i < number ;i++)
        {
           
            int sockfd = (epoll_object.getevents())[i].data.fd;
            socket_object.Setconnfd(sockfd);
            if(sockfd == socket_object.getsockfd())//连接的socket
            {
                bool connstate =  socket_object.Accept();
                if(connstate == false)
                {
                  my_err("accept connstate failed",__LINE__);
                  continue;
                }
                int connfd = socket_object.getconnfd();
                
                epoll_object.Addevent(&socket_object,true);
            }
            else if((epoll_object.getevents())[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                fprintf(stderr, "CLOSE CONNECTION");//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                socket_object.Closeclient();
            }
            else if((epoll_object.getevents())[i].events & EPOLLIN)//读
            {
                fprintf(stderr, "EPOLLIN");//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                // char *RECVBUF = new char[1000];
                // if(socket_object.Recv(RECVBUF,1000,-1))
                // {
                    // task *ta = new task(socket_object.getconnfd());
                    // ta();
                    pool.append(task(socket_object));
                        // [] (task a) { a.doit(); },
                        // task(socket_object));
                    // pool.append(ta);
                // }
                // else 
                    // socket_object.Closeclient();
                // delete[] RECVBUF;
            }
            else if((epoll_object.getevents())[i].events & EPOLLOUT)//写
            {
                if(socket_object.Send("1000",3))
                    socket_object.Closeclient();
            }
        }
       
    }
    //析构函数关epollfd
    socket_object.Closeserv();
   //怎么删除线程池

    return 0;
}
