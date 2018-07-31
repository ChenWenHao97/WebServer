#include<iostream>
#include"../utils/my_err.hpp"
#include"../utils/serversocket.hpp"
#include"../utils/threadpool.hpp"
#include"../utils/Epoll.hpp"
#include"../utils/task.hpp"
#include<unordered_map>
using namespace std;
unordered_map<string, string> ContentTypes;
void initContenttypeMap()
 {
    FILE *fp = fopen("../protocol/content_type.txt", "r");

    do {
        char key[20], value[50];
        fscanf(fp, " %s %s", key, value);
        ContentTypes[key] = value;
    } while (!feof(fp));

    fclose(fp);
}

int main(int argc,char *argv[])
{

    if(argc != 3)
    {
        cout <<"usage :"<<argv[0]<<"port"<<endl;
        return 1;
    }
    initContenttypeMap();
    std::cout << 1 <<endl;
    ServSocket socket_object;
   

    threadpool pool(20);//线程池
    int port = atoi(argv[2]);
    string ip = argv[1];
    socket_object.Socket(port,ip);
    Epoll epoll_object(1000);
    bool a = epoll_object.Addevent(&socket_object,false);

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
                socket_object.Closeclient();
            }
            else if((epoll_object.getevents())[i].events & EPOLLIN)//读
            {
                    pool.append(task(&socket_object));
            }
        }
       
    }
    //析构函数关epollfd
    socket_object.Closeserv();

    return 0;
}
