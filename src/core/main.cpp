#include "../utils/serversocket.hpp"
#include "../utils/threadpool.hpp"
#include "../utils/Epoll.hpp"
#include "../utils/task.hpp"
#include "../utils/init.hpp"
using namespace std;
extern unordered_map<string, string> ContentTypes;

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        cout << "usage :" << argv[0] << "port" << endl;
        return 1;
    }
    Init::initContenttypeMap();

    ServSocket socket_object;
    threadpool pool(20); //线程池

    int port = atoi(argv[2]);
    string ip = argv[1];
   
    socket_object.Socket(port, ip);
    socket_object.Setoptions(SO_REUSEADDR, 1); //设置reuseaddr,防止time_wait
    Epoll epoll_object(1000);
    bool a = epoll_object.Addevent(&socket_object, false);

    while (true)
    {
        int number = epoll_object.Waitevent();

    
        if (number < 0 && errno != EINTR)
        {
            Init::my_err("get epoll number failed", __LINE__);
            break;
        }

        for (int i = 0; i < number; i++)
        {

            int sockfd = (epoll_object.getevents())[i].data.fd;
            socket_object.Setconnfd(sockfd);
            if (sockfd == socket_object.getsockfd()) //连接的socket
            {

                bool connstate = socket_object.Accept();
                epoll_object.Addevent(&socket_object, true);
                if (connstate == false)
                {
                    Init::my_err("accept connstate failed", __LINE__);
                    continue;
                }
            }
            else if ((epoll_object.getevents())[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                cout <<"close client epollrdhup"<<endl;
                socket_object.Closeclient();
            }
            else if ((epoll_object.getevents())[i].events & EPOLLIN) //读
            {
          
                pool.append(task(&socket_object));
            }
        }
    }
    //析构函数关epollfd
    socket_object.Closeserv();

    return 0;
}
