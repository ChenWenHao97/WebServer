//半同步半异步线程池，有主线程(epoll等)
//领导者追随者没有主线程
#pragma once
#include "init.hpp"
using namespace std;

class threadpool
{
  private:
    using Task = function<void(void)>;
    vector<thread> pool;          //线程池
    queue<Task> tasks;            //任务队列
    mutex m_lock;                 //锁，保证任务删除和添加互斥性
    condition_variable condition; //阻塞条件
    atomic<bool> stopped;
    atomic<int> free_thread_num;

  public:
    inline threadpool(int size = 10) : stopped(false)
    {
        free_thread_num = (size < 1) ? 10 : size;
        for (int i = 0; i < free_thread_num; i++)
        {
            //创建线程
            pool.emplace_back(
                [this] //捕获当前类的指针，可以访问private
                {
                    function<void(void)> task;
                    while (!(this->stopped).load())
                    {
                        //获取一个待执行的task

                        unique_lock<mutex> lock(this->m_lock); //加锁
                        this->condition.wait(lock,
                                             [this] {
                                                 return (this->stopped).load() ||
                                                        !(this->tasks).empty();
                                             });
                        if ((this->stopped).load() && (this->tasks).empty())
                            //停止和队列为空就结束
                            return;

                        task = move((this->tasks).front());
                        //无论队列数据是什么参数，都可以取出来进行执行
                        (this->tasks).pop();

                        free_thread_num--;
                        task();
                        free_thread_num++;
                    }
                });
        }
    }
    inline ~threadpool()
    {
        stopped.store(true);
        condition.notify_all(); //唤醒所有线程
        for (thread &i : pool)
        {
            //i.detach();//自行结束
            if (i.joinable())
                i.join(); //等待任务结束,前提是线程一定执行完
        }
    }

    template <class Function, class... Args>
    auto append(Function &&f, Args &&... args) -> future<decltype(f(args...))>
    {
        if (stopped.load())
            throw runtime_error("append threadpool is closed");
        using T = decltype(f(args...));
        auto task = make_shared<packaged_task<T()>>(
            //返回智能指针
            //T只是返回值，（）代表不需要额外参数，变参必须都给

            bind(forward<Function>(f), forward<Args>(args)...)
            //参数预绑定，同时不改变左右值
            //相当于提前给了一个参数

        );
        future<T> future_results = task->get_future();
        {
            unique_lock<mutex> lock(m_lock);
            tasks.emplace(
                [task] {
                    (*task)();
                });
        }
        condition.notify_one(); //唤醒一个线程去执行
        return future_results;
    }

    int threadcount()
    {
        return free_thread_num;
    }
};

// string fun(std::string s)
// {
//     /*
//     1. read request
//     2. parse
//     3. -----> deal
//     4. return
//     */
//     return "str is:"+s;
// }

// int main()
// {

//     threadpool excute(20);
//     future<string> ff = excute.append(fun,"hello");
//     cout << ff.get().c_str()<<endl;

//     return 0;

// }
